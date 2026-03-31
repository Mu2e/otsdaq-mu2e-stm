// ===== Query param forwarding =====
const params = window.location.search;

if (params) {
    document.querySelectorAll("a").forEach(link => {
        link.href += params;
    });
}

// ===== Fetch JSON =====
async function fetchSTM() {
    const res = await fetch("../json/stm.json" + params);
    return await res.json();
}

// ===== Card helper =====
function setCard(id, value, state="ok") {
    const el = document.getElementById(id);
    if (!el) return;

    const card = el.closest(".card");

    el.textContent = value;

    card.classList.remove("ok", "warn", "bad");
    card.classList.add(state);
}

// ===== Binary state helper =====
function stateBinary(val) {
    return val > 0 ? "bad" : "ok";
}

// ===== RUN TRACKING =====
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

// ===============================
// GLOBAL ALARM AUDIO
// ===============================

const alarmAudio = new Audio("../audio/alarm.mp3");
alarmAudio.loop = true;

let alarmActive = false;
let muted = localStorage.getItem("alarmMuted") === "true";

// ===============================
// CONTROL FUNCTIONS
// ===============================

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

// ===============================
// ALARM LOGGING
// ===============================
// ===============================
// GLOBAL ALARM LOGGING (WITH EWT)
// ===============================

function logAlarms(data) {

    const alarms = data.alarms;
    const ewt    = data.timing?.ewt ?? null;

    let history = JSON.parse(localStorage.getItem("alarmHistory") || "[]");
    let prev    = JSON.parse(localStorage.getItem("lastAlarms") || "{}");

    const now = new Date().toISOString();

    for (const [key, val] of Object.entries(alarms)) {

        // Rising edge only
        if (val === true && prev[key] !== true) {

            history.push({
                alarm: key,
                time: now,
                ewt: ewt,
                run: data.run ?? null
            });
        }
    }

    // Limit size
    if (history.length > 500) {
        history = history.slice(history.length - 500);
    }

    localStorage.setItem("alarmHistory", JSON.stringify(history));
    localStorage.setItem("lastAlarms", JSON.stringify(alarms));
}

// ===============================
// HELPERS
// ===============================

function formatAlarm(name) {
    return name
        .replace(/_/g, " ")
        .replace(/\b\w/g, c => c.toUpperCase());
}

// ===============================
// SOUND TOGGLE (LUCIDE)
// ===============================

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

// ===============================
// THEME TOGGLE (LUCIDE)
// ===============================

function updateThemeUI() {
    const icon = document.getElementById("themeIcon");
    const text = document.getElementById("themeText");

    if (!icon || !text) return;

    const isLight = document.body.classList.contains("light");

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
    if (theme === "light") {
        document.body.classList.add("light");
    } else {
        document.body.classList.remove("light");
    }
}

function toggleTheme() {
    document.body.classList.toggle("light");

    const isLight = document.body.classList.contains("light");
    localStorage.setItem("theme", isLight ? "light" : "dark");

    if (window.charts) {
        window.charts.forEach(c => applyThemeToChart(c));
    }

    updateThemeUI();
}

// ===============================
// ALARMS
// ===============================

function renderAlarms(containerId, alarms, keys) {
    const el = document.getElementById(containerId);
    if (!el) return;

    let html = "";

    for (const key of keys) {
        const val = alarms[key];
        const cls = val ? "bad" : "ok";

        const icon = val ? "alert-triangle" : "check-circle";

        html += `
            <div class="${cls}">
                <i data-lucide="${icon}"></i>
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

    const alarms = data.alarms;

    const badKeys = Object.entries(alarms)
        .filter(([k, v]) => v === true)
        .map(([k]) => formatAlarm(k));

    const hasAlarm = badKeys.length > 0;

    // ===== Run =====
    if (data.run !== undefined) {
        setRun(data.run);
    }
    const run = data.run ?? getRun() ?? "N/A";

    // ===== Spill =====
    const isOnSpill = data.timing?.on_spill;

    const spillText = isOnSpill ? "ON SPILL" : "OFF SPILL";
    const spillClass = isOnSpill ? "spill-on" : "spill-off";

    // ===== Time =====
    const time = new Date().toLocaleTimeString();

    banner.classList.remove("ok", "bad");

    if (hasAlarm) {
        banner.classList.add("bad");

        banner.innerHTML = `
            <i data-lucide="alert-triangle"></i>
            Run ${run} | <span class="${spillClass}">${spillText}</span> | ${badKeys.join(" | ")} 
            <span class="time">(${time})</span>
        `;

        playAlarm();

    } else {
        banner.classList.add("ok");

        banner.innerHTML = `
            <i data-lucide="check-circle"></i>
            Run ${run} | <span class="${spillClass}">${spillText}</span> | System OK 
            <span class="time">(${time})</span>
        `;

        stopAlarm();
    }

    localStorage.setItem("alarmActive", hasAlarm);
    logAlarms(data);

    lucide.createIcons();
}

// ===== Timestamp updater =====
function updateTimestamp() {
    const el = document.getElementById("lastUpdate");
    if (!el) return;

    const now = new Date();
    el.textContent = "Last update: " + now.toLocaleTimeString();
}

// ===============================
// INIT
// ===============================

(function () {
    const saved = localStorage.getItem("theme") || "dark";
    applyTheme(saved);
})();

window.addEventListener("load", () => {

    muted = localStorage.getItem("alarmMuted") === "true";

    updateSoundUI();
    updateThemeUI();

    if (localStorage.getItem("alarmActive") === "true") {
        playAlarm();
    }

    lucide.createIcons();
});

