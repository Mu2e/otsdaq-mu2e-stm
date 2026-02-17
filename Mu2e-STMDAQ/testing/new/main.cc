#include <cstdlib>
#include "gen_data.hh"  

// Main function
int main(int argc, char* argv[]){

  // Argument variables
  size_t event_num = 0;
  size_t event_len = 0;
  std::string filename = "";

  // Print to user
  std::cout << "Executing packet generation programme with the following " << argc<< " inputs: " << std::endl;
  // If the accepted number of arguments
  if (argc == 3 or argc == 4){
    event_num = std::stoull(argv[1], nullptr, 0); // Get event number
    event_len = std::stoull(argv[2], nullptr, 0); // Get event lengh
    if (argc == 4) filename = argv[3];
    std::cout << "Number of events = " << event_num << "." << std::endl;
    std::cout << "Event length = " << event_len << " ns." << std::endl;
    if (argc == 3){
      std::cout << "Incrememting counter ADC data will be generated." << std::endl;
    }
    else{
      std::cout << "Input binary file = " << filename << "." << std::endl;
    }      
  }
  else{
    std::cout << "Incorrrect argument input. Please re-run with the following list:" << std::endl;
    std::cout << "./run.exe [number of events] [event length (ns)] [input binary file (OPTIONAL)]" << std::endl;
    return 0;
  }
  
  // Infinite loop for user inut
  while (true) {
    char input;
    std::cout << "Continue? Input Y or N to continue: ";
    std::cin >> input;
    if (input == 'y' or input == 'Y') {	  
      std::cout << "Continuing..." << std::endl;
      break;
    }
    else if (input == 'n' or input == 'N') {
      std::cout << "You chose not to continue. Exiting..." << std::endl;
      return 0;
    }
    else {
      std::cout << "Invalid input. Please enter Y or N." << std::endl;
      // Clear the fail state and discard the rest of the input line
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
  }
  
  std::cout << "Acquiring data and storing in memory..." << std::endl;
  
  // Generate data
  std::shared_ptr<GenData> gen = std::make_shared<GenData>(event_num,event_len,filename);

  // Wait for user to continue
  std::cout << "Press any key to continue...";
  std::cin.get();
  // Ignore the newline character left in the input buffer
  std::cin.ignore();
  std::cout << "Continuing..." << std::endl;
  
  // // Send client socket (mimics fw)
  // int send_sock = udp.setupClient();
  
  // // Start send thread
  // std::thread send_thread(&test_funcs::client,
  // 			  std::ref(testing),
  // 			  std::ref(udp),
  // 			  send_sock,
  // 			  send_packets,
  // 			  packetNum,
  // 			  numberhb,
  // 			  send_num);

  // // Join thread
  // send_thread.join();

  return 0;
  
}

 
