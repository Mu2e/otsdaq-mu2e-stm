import numpy as np
import psycopg2
from collections import defaultdict
import datetime,time

class Baseline:

    def write_run_subrun(self,run,subrun,start_time,end_time):
        sql = "insert into run_subrun_times (run,subrun,start_time,end_time) values (%d,%d,'%s','%s')" % (run,subrun,start_time,end_time)
        print(sql)
        self.curr.execute(sql)
        self.conn.commit()

    def write_data(self,run,subrun,mean,rms):
        sql = "insert into hpge_baseline (run,subrun,mean,rms) values (%d,%d,%.4f,%.4f)" % (run,subrun,mean,rms)
        print(sql)
        self.curr.execute(sql)
        self.conn.commit()

    def RunSubRunTime(self,run,subrun):  # return time as datetime of start of sub run
#        print("RunSubRunTime(run=%d,subrun=%d)" % (run,subrun))
#        print(self.runTimes)
        if (run in self.runTimes):
            d = self.runTimes[run]
#            print("... run found...",d)
            st = d['StartTimes']
            start_time = st[subrun]
#            et = d['EndTimes']
#            end_time   = et[subrun]
            return datetime.datetime.fromtimestamp(start_time)
        else:
            return None

    def StartEndTime(self,run):
        if (run in self.runTimes):
            d = self.runTimes[run]
            st = d['StartTime']
            et = d['EndTime']
            return datetime.datetime.fromtimestamp(st),datetime.datetime.fromtimestamp(et),et-st
        else:
            return None, None, None


    def getRunTimes(self,runMin,runMax):
    # start time and end time for each run,subrun
        sql = "select run,subrun,start_time,end_time from run_subrun_times where run >= %d and run <= %d and start_time > '1970-01-01 00:00:00' and end_time > '1970-01-01 00:00:00' order by run,subrun" % (runMin,runMax) 
        self.curr.execute(sql)
        self.conn.commit()
        rows = self.curr.fetchall()

        runTimesX = defaultdict(int)
        for row in rows:
            run = int(row[0])
            subrun = int(row[1])
            start_time = int(time.mktime(row[2].timetuple()))
            end_time = int(time.mktime(row[3].timetuple()))

            if (run not in runTimesX):
                d = {'NumSubRuns' : 0, 'StartTimes' : defaultdict(int), 'EndTimes' : defaultdict(int) }
                runTimesX[run] = d

            d = runTimesX[run]
            d['NumSubRuns'] = d['NumSubRuns'] + 1

            st = d['StartTimes']
            et = d['EndTimes']
            st[subrun] = start_time
            et[subrun] = end_time 

        return runTimesX



    def __init__(self):
# Connect to Postgres DB
        dsn  = "dbname=stm user=stm_writer password=stm_writer host=localhost port=5432"
        self.conn = psycopg2.connect(dsn)
        self.curr = self.conn.cursor()
        self.runTimes = self.getRunTimes(0,100000)  # get times for all runs,subruns
#        print(self.runTimes)


    def get_data_run(self,run=None):
        if (run is not None):
            sql = "select run, subrun, mean, rms from hpge_baseline where run = %d" % (run)
        else:
            sql = "select run, subrun, mean, rms from hpge_baseline where run > 0"
#        print(sql)
        self.curr.execute(sql)
        self.conn.commit()
        rows = self.curr.fetchall()
        mean_values = np.array([])
        rms_values = np.array([])
        time_values = np.array([])
#        print(rows)
        for row in rows:
            run = int(row[0])
            subrun = int(row[1])
            mean_values = np.append(mean_values,float(row[2]))
            rms_values = np.append(rms_values,float(row[3]))
#            print("calling self.RunSubRunTime(%d,%d)" % (run,subrun))
            subrun_time = self.RunSubRunTime(run,subrun)  # return time (as datetime) of start of sub run
            time_values = np.append(time_values,subrun_time)

        return mean_values, rms_values, time_values

    def close(self):
        self.conn.close()

