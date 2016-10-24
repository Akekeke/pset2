#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

//unordered_map std::map<uint64_t,uint64_t> unacked;
//unordered_map std::map<uint64_t,uint64_t> acked;
//unordered_map std::map<uint64_t,uint64_t> rtt;

//std::map<uint64_t, uint64_t>::iterator it;

float the_window_size = 50;
float thresh = 100;



/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  {}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }
  return (unsigned int) the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( 
            const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */
  //uint64_t rtt = 0;
  //acked[sequence_number] = timestamp_ack_received;
  //RTT = acked[sequence_number] - unacked[sequence_number];

  float RTT = float(timestamp_ack_received) - float(send_timestamp_acked);

  //unsigned int a = 1; //additive increase for window size
  float min_ws = 1;

  float max_thrs = 1000;
  float min_thrs = 1; 
  //uint64_t b = 2; //additive decrease for theshold  
  float c = 2; //multiplicative decrease for window size 
  //float d = 2; //multiplicative increase for threshold

  fprintf(stderr, "RTT = %f \n", RTT);
  if (RTT<thresh){
    the_window_size = the_window_size + 1;
    thresh = thresh-10;
    fprintf(stderr, "IF window size = %f, and threshold = %f \n", the_window_size,thresh);
  }
  else{
    the_window_size = std::max(the_window_size/c,min_ws);
    thresh = thresh+100;
    fprintf(stderr, "ELSE window size = %f, and threshold = %f \n", the_window_size,thresh);
  }
  thresh = std::max(thresh,min_thrs);
  thresh = std::min(thresh,max_thrs);


  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}