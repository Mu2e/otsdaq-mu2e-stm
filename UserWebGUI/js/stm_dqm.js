// ===== Query param forwarding =====
document.addEventListener("DQMContentLoaded", () => {
    const params = window.location.search;

    if (params) {
        document.querySelectorAll("a").forEach(link => {
            link.href += params;
        });
    }
});

// ===== GLOBAL THRESHOLDS =====
const DQM_THRESHOLDS = {

    timing: {
        ewt_small_warn: 1,
        ewt_small_bad: 10,

        ewt_trans_warn: 1,
        ewt_trans_bad: 10,

        dtc_warn: 1,
        dtc_bad: 2000,

        adc_warn: 1,
        adc_bad: 500
    },

    rate: {
        min: 1
    },

    correlation: {
        slope_min: 4000,
        slope_max: 7000,
        stability_warn: 500
    },

    integrity: {
        no_events_warn: true,
        stalled_bad: true
    },

    detector: {
        chi2_warn: 5,
        chi2_bad: 10,

        peak_shift_warn: 5,
        peak_shift_bad: 15
    }

};

// ===== Data status (GLOBAL) =====
function getDataAgeSeconds(data) {
    if (!data || !data.timestamp) return Infinity;
    return Date.now() / 1000 - data.timestamp;
}

let stoppedSince = null;

function isSystemStopped(data, timeoutSec = 45, refSec = 60) {

    const age = getDataAgeSeconds(data);

    if (age <= timeoutSec) {
        stoppedSince = null;
        return false;
    }

    if (stoppedSince === null) {
        stoppedSince = Date.now();
        return false;
    }

    const staleFor = (Date.now() - stoppedSince) / 1000;

    return staleFor > refSec;
}

function getSystemState(data) {
    if (!data) return "unknown";
    if (isSystemStopped(data)) return "stopped";
    if (getActiveAlarms(data).length > 0) return "alarm";
    return "ok";
}

// ===== Detector =====
function getDetectorHealth(data) {
    const spec = data.spectrum ?? {};
    const peaks = spec.peaks ?? [];
    const cal = spec.calibration ?? {};
    const chi2 = spec.chi2_ndf ?? 0;

    const cs = peaks.find(p => p.name === "Cs137");

    return {
        calibration: cal.a ?? null,
        resolution: spec.resolution?.A ?? null,
        peakShift: cs ? Math.abs(cs.mean - 600) : null,
        chi2: chi2,

        states: {
            calibration: (cal.a && Math.abs(cal.a - 1.1) > 0.1) ? "warn" : "ok",
            peakShift: (cs && Math.abs(cs.mean - 600) > 5) ? "warn" : "ok",
            chi2: chi2 > 2 ? "warn" : "ok"
        }
    };
}

// ===== Threshold helper =====
function getState(val, warn, bad) {
    if (val >= bad) return "bad";
    if (val >= warn) return "warn";
    return "ok";
}

// ===== Alarm severity =====
function getAlarmSeverity(alarm) {

    switch (alarm) {
        case "clock_error":
        case "correlation_error":
            return "bad";

        case "ewt_error":
        case "transition_error":
            return "warn";

        case "cleared":
            return "ok";
    }

    if (alarm.includes("clock") || alarm.includes("correlation")) return "bad";
    if (alarm.includes("ewt") || alarm.includes("transition")) return "warn";

    return "ok";
}

// ===== Detector detection =====
function getDetector() {
    if (window.location.pathname.toLowerCase().includes("labr")) return "labr";
    return "hpge";
}

// ===== JSON path (auto depth-safe) =====
function getJSONPath(detector) {
    const path = window.location.pathname.toLowerCase();

    if (path.includes("/hpge/") || path.includes("/labr/")) {
        return `../../json/stm_${detector}.json`;
    }

    return `../json/stm_${detector}.json`;
}

// ===== Fetch JSON (cache-safe) =====
async function fetchSTM(detector = getDetector()) {
    const path = getJSONPath(detector);
    const res = await fetch(path + "?_=" + Date.now());
    return await res.json();
}

// ===== Alarm helper =====
function getActiveAlarms(data) {
    return Object.entries(data.alarms || {})
        .filter(([_, v]) => v === true)
        .map(([k]) => k);
}

// ===== Card helper =====
function setCard(id, value, state="ok") {
    const el = document.getElementById(id);
    if (!el) return;

    const card = el.closest(".card");

    el.textContent = value;

    if (!card) return;

    card.classList.remove("ok", "warn", "bad");
    card.classList.add(state);
}

// ===== Binary state helper =====
function stateBinary(val) {
    return val > 0 ? "bad" : "ok";
}

// ===== Run Tracking =====
function getRun() {
    return localStorage.getItem("currentRun");
}

function setRun(run) {
    localStorage.setItem("currentRun", run);
}

function checkRunChange(newRun) {
    const oldRun = getRun();

    if (oldRun === null) {
        setRun(newRun);
        return false;
    }

    if (String(oldRun) !== String(newRun)) {
        localStorage.setItem("previousRun", oldRun);
        setRun(newRun);
        return true;
    }

    return false;
}

function getBasePath() {
    const path = window.location.pathname.toLowerCase();

    if (path.includes("/hpge/") || path.includes("/labr/")) {
        return "../../";
    }

    return "../";
}

// ===== Alarm audio =====
const alarmAudio = new Audio(getBasePath() + "audio/alarm.mp3");
alarmAudio.loop = true;

let alarmActive = false;
let muted = localStorage.getItem("alarmMuted") === "true";

// ===== Alarm control =====
function playAlarm() {
    if (muted) return;

    if (!alarmActive) {
        alarmAudio.currentTime = 0;
        alarmAudio.play().catch(() => {});
        alarmActive = true;
    }
}

function stopAlarm() {
    if (alarmActive) {
        alarmAudio.pause();
        alarmAudio.currentTime = 0;
        alarmActive = false;
    }
}

// ===== Alarm logs =====
function logAlarms(data) {

    const alarms = data.alarms || {};
    const ewt    = data.window?.last_ewt ?? null;

    let history = JSON.parse(localStorage.getItem("alarmHistory") || "[]");
    let prev    = JSON.parse(localStorage.getItem("lastAlarms") || "{}");

    const now = new Date().toISOString();

    for (const [key, val] of Object.entries(alarms)) {

        if (val === true && prev[key] !== true) {

            history.push({
                alarm: key,
                time: now,
                ewt: ewt,
                run: data.run ?? null,
                detector: getDetector()
            });
        }
    }

    if (history.length > 500) {
        history = history.slice(history.length - 500);
    }

    localStorage.setItem("alarmHistory", JSON.stringify(history));
    localStorage.setItem("lastAlarms", JSON.stringify(alarms));
}

// ===== Alarm helpers =====
function formatAlarm(name) {

    const map = {
        ewt_error: "EWT Error",
        transition_error: "Spill Transition Error",
        clock_error: "Clock Drift",
        correlation_error: "Timing Correlation",
        cleared: "Cleared"
    };

    let label = map[name] || name
        .replace(/_/g, " ")
        .replace(/\b\w/g, c => c.toUpperCase());

    return label;
}

// ===== Sound UI =====
function toggleMute() {
    muted = !muted;
    localStorage.setItem("alarmMuted", muted);

    if (muted) stopAlarm();

    updateSoundUI();
}

function updateSoundUI() {
    const icon = document.getElementById("soundIcon");
    const text = document.getElementById("soundText");
    const btn  = document.getElementById("soundToggle");

    if (!icon || !text || !btn) return;

    btn.classList.remove("active", "muted");

    if (muted) {
        icon.setAttribute("data-lucide", "volume-x");
        text.textContent = "Sound OFF";
        btn.classList.add("muted");
    } else {
        icon.setAttribute("data-lucide", "volume-2");
        text.textContent = "Sound ON";
        btn.classList.add("active");
    }

    lucide.createIcons();
}

// ===== Theme =====
function updateThemeUI() {
    const icon = document.getElementById("themeIcon");
    const text = document.getElementById("themeText");

    if (!icon || !text) return;

    const isLight = document.documentElement.classList.contains("light");

    if (isLight) {
        icon.setAttribute("data-lucide", "sun");
        text.textContent = "Light Mode";
    } else {
        icon.setAttribute("data-lucide", "moon");
        text.textContent = "Dark Mode";
    }

    lucide.createIcons();
}

function applyTheme(theme) {
    const root = document.documentElement;

    if (theme === "light") {
        root.classList.add("light");
    } else {
        root.classList.remove("light");
    }
    updateThemeUI();
}

function toggleTheme() {
    const root = document.documentElement;

    root.classList.toggle("light");

    const isLight = root.classList.contains("light");
    localStorage.setItem("theme", isLight ? "light" : "dark");

    if (window.charts) {
        window.charts.forEach(c => applyThemeToChart(c));
    }

    updateThemeUI();
}

// ===== Alarm rendering =====
function renderAlarms(containerId, alarms, keys) {
    const el = document.getElementById(containerId);
    if (!el) return;

    let html = "";

    for (const key of keys) {
        const val = alarms[key];
        const cls = val ? getAlarmSeverity(key) : "ok";

        html += `
            <div class="${cls}">
                ${formatAlarm(key)}: ${val}
            </div>
        `;
    }

    el.innerHTML = html;

    lucide.createIcons();
}

// ===== Banner updater =====
function updateBanner(data) {
    const banner = document.getElementById("banner");
    if (!banner) return;

    const stopped = isSystemStopped(data);

    if (stopped) {
        banner.classList.remove("ok", "bad");
        banner.classList.add("warn");

        banner.innerHTML = `
            DATA STOPPED | Last update ${Math.floor(getDataAgeSeconds(data))}s ago
        `;

        stopAlarm();
        lucide.createIcons();
        return;
    }

    const badKeys = getActiveAlarms(data);
    const hasAlarm = badKeys.length > 0;

    if (data.run !== undefined) {
        setRun(data.run);
    }
    const run = data.run ?? getRun() ?? "N/A";
    const subrun = data.subrun ?? "N/A";

    const isOnSpill = data.spill?.end ?? false;
    const spillText = isOnSpill ? "ON SPILL" : "OFF SPILL";
    const spillClass = isOnSpill ? "spill-on" : "spill-off";

    const time = new Date().toLocaleTimeString();

    banner.classList.remove("ok", "bad");

    if (hasAlarm) {
        banner.classList.add("bad");

        banner.innerHTML = `
            Run ${run}.${subrun} | <span class="${spillClass}">${spillText}</span> | ${badKeys.map(formatAlarm).join(" | ")}
            <span class="time">(${time})</span>
        `;

        playAlarm();
    } else {
        banner.classList.add("ok");

        banner.innerHTML = `
            Run ${run}.${subrun} | <span class="${spillClass}">${spillText}</span> | System OK
            <span class="time">(${time})</span>
        `;

        stopAlarm();
    }

    localStorage.setItem("alarmActive", hasAlarm);
    logAlarms(data);

    lucide.createIcons();
}

// ===== Timestamp =====
function updateTimestamp() {
    const el = document.getElementById("lastUpdate");
    if (!el) return;

    el.textContent = "Last update: " + new Date().toLocaleTimeString();
}

window.addEventListener("load", () => {

    muted = localStorage.getItem("alarmMuted") === "true";

    updateSoundUI();
    updateThemeUI();

    if (localStorage.getItem("alarmActive") === "true") {
        playAlarm();
    }

    lucide.createIcons();
});
