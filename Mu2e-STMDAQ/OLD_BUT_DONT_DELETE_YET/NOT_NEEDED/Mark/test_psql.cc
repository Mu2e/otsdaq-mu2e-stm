#include <iostream>
#include "STMDAQ-TestBeam/utils/dbase.hh"
#include <map>
#include <algorithm>
#include <midas.h>
#include "unistd.h"

int main(int argc, char* argv[]) {

    DBase *db = new DBase("127.0.0.1",5432,DBase::WRITE);
    db->write_hpge_baseline(39,0,100.12, 10.23); // run,subrun,mean,rms
    db->update_hpge_baseline(39,0,101.12, 11.23); 
    std::map<std::string, float> results = db->read_hpge_baseline(39,0);

    if (results.size() == 2) {
      std::cout << "Mean for this run, subrun = " << results["mean"] << " and rms = " << results["rms"] << std::endl;
    }
    else if (results.size() == 0) {
      std::cout << "*** No baseline values for this run, subrun" << std::endl;
    }
    else {
      std::cout << "*** Expected two values but got : " << results.size() << std::endl;
    }

    /* Example code to get run start comment from MIDAS ODB and out in SQL database
    char startComment[256]; int size = sizeof(startComment);
    // extern HNDLE hDB;
    db_get_value(hDB,0,"/Experiment/Edit on start/Comment",&startComment,&size,TID_STRING,FALSE);
    std::string comment(startComment);
    // Remove any quotes from comment string so it doesn't shaft the SQL quoting...
    comment.erase(std::remove(comment.begin(),comment.end(),'\"'),comment.end());
    comment.erase(std::remove(comment.begin(),comment.end(),'\''),comment.end());
    db->write_runinfo_start(1,comment);
    */

    db->write_runinfo_start(1,"Run-1 test comment");
    sleep(5);
    db->write_runinfo_stop(1);
    db->con()->disconnect();

    delete db;

}
