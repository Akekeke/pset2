#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

float wind_sz = 50;
uint64_t last_ack_received = 0;
uint64_t min_RTT = 100; //initial estimate of min RTT is 100 ms
float min_wind = 1;
float max_wind = 35;
float interarrival_avg = 1; 
/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  wind_sz = std::max(min_wind,std::min(wind_sz,max_wind));
  
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << (unsigned int) wind_sz << endl;
  }

  return (unsigned int) wind_sz;
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
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */
  float b = 0.5;
  float d = 0.5;
  float min_queue_size = 10;
 
  uint64_t rtt_est = timestamp_ack_received - send_timestamp_acked; //new rtt estimate
  
  if (rtt_est < min_RTT){
    min_RTT = rtt_est;
  }
  
  uint64_t iat = timestamp_ack_received - last_ack_received;//interarrival time
//  fprintf(stderr, "iat: %f",iat);
	uint64_t diff_rtt = (uint64_t)std::abs(long(rtt_est - min_RTT));
  if (iat<1){
   iat = 1;
  }

  double interarrival_ave_update = interarrival_avg * (1.0-d) + iat * (d);
  if (diff_rtt < 5) { //essentially, no difference between rtt_est and min_RTT, so queue is none
    fprintf(stderr, "NO QUEUE DETECTED.\n" );
    wind_sz = wind_sz + (float)1.0/wind_sz;
  } 
  else{
     if (iat > interarrival_avg) {
      interarrival_avg = iat;
    } else {
      interarrival_avg = interarrival_ave_update;
    }
    wind_sz = b*float(min_RTT)/float(interarrival_avg) + min_queue_size;
//    fprintf(stderr, "Interarrival_avg: %f, wind_sz: %f \n",interarrival_avg,wind_sz);
  }
    
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
  
  last_ack_received = timestamp_ack_received;
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
