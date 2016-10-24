#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

float wind_sz = 50;
float min_wind = 1;
float max_wind = 35;
float target_latency = 100;
float packets_sent = 0;
float packets_recvd = 0;
uint64_t rtt_est = 100;
uint64_t prev_ack_sent = 0;
uint64_t prev_ack_rec = 0;

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
  packets_sent+=1.0;
  
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
 
  float num_outstanding = packets_sent - packets_recvd;
  packets_recvd +=1.0;
  
  rtt_est = (rtt_est+prev_ack_rec - prev_ack_sent)/2; //new rtt estimate
  
  prev_ack_rec=timestamp_ack_received;
  prev_ack_sent=send_timestamp_acked;
  float bandwidth_est = num_outstanding/rtt_est;
  
  float capacity = target_latency*bandwidth_est;

  float processed_pkts = rtt_est/2*bandwidth_est;
  capacity+=processed_pkts;
  
  if (capacity > wind_sz){
    wind_sz += 1/wind_sz;   
  } else{
    wind_sz = wind_sz - 5;
  }
  
    
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
