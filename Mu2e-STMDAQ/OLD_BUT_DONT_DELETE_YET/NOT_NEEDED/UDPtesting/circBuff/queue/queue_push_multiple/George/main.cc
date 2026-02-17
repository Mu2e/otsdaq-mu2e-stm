#include "queue.hh"

int main(){

  queue_buffer<int> order_up;

  int chan = 1;  
  int* pushData = new int[m_num];
  int* pullData = new int[m_num]();
  int n = m_num;
  srand((unsigned) time(NULL));

  for(int i=0; i<m_num; i++){
    int random = 1 + (rand() % m_num);
    pushData[i] = random;
  }

  std::thread *pushT = new std::thread(pushFn, chan, pushData, push_max);
  std::thread *pullT = new std::thread(pullFn, chan, pullData);
  pushT->join();
  pullT->join();
  int x=0;
  bool success = true;
  for(int i=0; i < m_num; i++){
    if(pushData[i] != pullData[i]) success = false;
  }
  if(success){
    cout << "\n Successfully pushed all events \n" << endl;
  }
  else{
    cout << "\n Job failed \n" << endl;
  }

  return 1;
}
