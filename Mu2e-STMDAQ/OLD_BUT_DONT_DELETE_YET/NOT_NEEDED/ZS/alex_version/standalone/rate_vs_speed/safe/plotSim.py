import csv
rates_all = []
triggers = []
speed = []
with open('662keV_0.32mV.txt') as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        rates_all.append(float(row[0]))
        triggers.append(float(row[1]))
        speed.append(float(row[2]))

rates_tmp = set(rates_all)
rates = (list(rates_tmp))
rates.sort()
print(rates)
trigger_avg = list(range(len(rates)))
speed_avg = list(range(len(rates)))
for i in range(len(rates)):
    trigger_avg[i] = 0
    speed_avg[i] = 0
    for j in range(len(rates_all)):
        if (rates[i] == rates_all[j]):
            trigger_avg[i] += triggers[j]
            print (rates[i], rates_all[j], triggers[j], trigger_avg[i])
            speed_avg[i] += speed[j]
for i in range(len(rates)):
    trigger_avg[i] /= 10
    speed_avg[i] /= 10

print (trigger_avg,speed_avg)

import matplotlib.pyplot as plt

fig, ax1 = plt.subplots()

ax2 = ax1.twinx()
ax1.plot(rates, trigger_avg, 'g--')
ax2.plot(rates, speed_avg, 'b-')

ax1.set_xlabel('Pulse rate [kHz]')
ax1.set_ylabel('% triggers found by ZS algorithm', color='g')
ax2.set_ylabel('Average ZS algorithm speed (Gbit/s)', color='b')
plt.title("662keV_0.32mV") 
plt.savefig("662keV_0.32mV.pdf")
plt.savefig("662keV_0.32mV.png")
            
