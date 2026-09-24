import csv
rates_all = []
triggers = []
speed = []
size = []
peaks = [9987,9982,9980,10000,9999,10000,9999,10000,10000,10000,9999,9995,9999,9999,10000]
with open('662keV_0.32mV.txt') as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        rates_all.append(float(row[0]))
        triggers.append(float(row[1])*10000.0/100)
        speed.append(float(row[2]))
        size.append(float(row[3]))

rates_tmp = set(rates_all)
rates = (list(rates_tmp))
rates.sort()
print(rates)
trigger_avg = list(range(len(rates)))
speed_avg = list(range(len(rates)))
size_avg = list(range(len(rates)))
for i in range(len(rates)):
    trigger_avg[i] = 0
    speed_avg[i] = 0
    for j in range(len(rates_all)):
        if (rates[i] == rates_all[j]):
            trigger_avg[i] += triggers[j]
            print (rates[i], rates_all[j], triggers[j], trigger_avg[i])
            speed_avg[i] += speed[j]
            size_avg[i] += size[j]
for i in range(len(rates)):
    trigger_avg[i] /= 10
    trigger_avg[i] = (trigger_avg[i] / peaks[i])*100
    speed_avg[i] /= 10
    size_avg[i] /= 10

print (trigger_avg,speed_avg,size_avg)

import matplotlib.pyplot as plt

fig, ax1 = plt.subplots()

ax2 = ax1.twinx()
lns1 = ax1.plot(rates, trigger_avg, 'g--', label='X-ray pulses found by ZS algorithm')
lns2 = ax1.plot(rates, size_avg, 'g-.', label='Reduction in data size')
lns3 = ax2.plot(rates, speed_avg, 'b-', label='Average ZS algorithm speed')
ax1.tick_params(axis='y', colors='g')
ax2.tick_params(axis='y', colors='b')

ax1.set_xlabel('Pulse rate [kHz]')
ax1.set_ylabel('Percent (%)', color='g')
ax2.set_ylabel('Data processing rate (Gbit/s)', color='b')
# ax1.legend(framealpha=0.5, bbox_to_anchor=(0.97, 0.99), loc='upper right', frameon=False)
# ax2.legend(framealpha=0.5, bbox_to_anchor=(0.97, 0.91), loc='upper right', frameon=False)
# added these three lines
lns = lns1+lns2+lns3
labs = [l.get_label() for l in lns]
ax1.legend(lns, labs, loc='best')

plt.title("662keV_0.32mV") 
plt.savefig("662keV_0.32mV.pdf")
plt.savefig("662keV_0.32mV.png")
            
