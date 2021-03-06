#include "sfd.h"
#include "rng.h"
#include <stdint.h>
#include <algorithm>
#include <float.h>

static class SFDClass : public TclClass {
  public:
    SFDClass() : TclClass("Queue/SFD") {}
    TclObject* create(int, const char*const*) {
      return (new SFD);
    }
} class_sfd;

int SFD::command(int argc, const char*const* argv)
{
  if (argc == 3) {
    if (!strcmp(argv[1], "user_id")) {
      user_id=atoi(argv[2]);
      printf("Setting user_id to %d \n", user_id);
      return TCL_OK;
    }
  }
  return EnsembleAwareQueue::command(argc, argv);
}

SFD::SFD() :
  EnsembleAwareQueue(),
  _packet_queue( new PacketQueue() ),
  _dropper(),
  _rate_estimator()
{
  bind("_iter", &_iter );
  bind("_K", &_K );
  bind("_headroom", &_headroom );
  fprintf( stderr,  "SFD: _iter %d, _K %f, _headroom %f \n", _iter, _K, _headroom );
  _dropper.set_iter( _iter );
  _rate_estimator = FlowStats(_K);
}

void SFD::enque(Packet *p)
{
  /* Implements pure virtual function Queue::enque() */

  /* Estimate arrival rate with an EWMA filter */
  double now = Scheduler::instance().clock();
  double arrival_rate = _rate_estimator.est_arrival_rate(now, p);

  /* Estimate current link rate with an EWMA filter. */
  _scheduler->update_link_rate_estimate();
  auto current_link_rate = _rate_estimator.est_link_rate(now, _scheduler->get_link_rate_estimate(user_id));
  
  /* Divide Avg. link rate by # of active flows to get fair share */
  auto _fair_share = (current_link_rate * (1-_headroom)) / (_scheduler->num_active_users() == 0 ? 1 : _scheduler->num_active_users());
  //printf("User id is %d, _fair_share is %f \n", user_id, _fair_share);

  /* Print everything */
  //print_stats( now );

  /* Compute drop_probability */
  double drop_probability = (arrival_rate < _fair_share) ? 0.0 : 1.0 ;

  /* Check aggregate arrival rate and compare it to aggregate ideal pf throughput */
  bool exceeded_capacity = _scheduler->agg_arrival_rate() > _scheduler->agg_pf_throughput() ;

  /* Enque packet */
  _packet_queue->enque( p );
 
  /* Toss a coin and drop */
  if ( !_dropper.should_drop( drop_probability ) ) {
   // printf( " Time %f : Not dropping packet, from flow %u drop_probability is %f\n", now, user_id, drop_probability );
  } else if ( !exceeded_capacity ) {
   // printf( " Time %f : Not dropping packet, from flow %u agg ingress %f, less than capacity %f \n", now, user_id, _scheduler->agg_arrival_rate(), _scheduler->agg_pf_throughput() );
  } else {
    /* Drop from front of the same queue */
  //  printf( " Time %f : Dropping packet, from flow %u drop_probability is %f\n", now, user_id, drop_probability );
    Packet* head = _packet_queue->deque();
    if (head != 0 ) {
        drop( head );
    }
  }
}

Packet* SFD::deque()
{
  /* Implements pure virtual function Queue::deque() */
  double now = Scheduler::instance().clock();
  Packet *p = _packet_queue->deque();
  _rate_estimator.est_service_rate(now, p);
  //print_stats( now );
  return p;
}

void SFD::print_stats( double now )
{
  /* Queue sizes */
  printf(" Time %f : Q :  ", now );
  printf(" %u %d ", user_id, _packet_queue->length());
  printf("\n");

  /* Arrival, Service, fair share, and ingress rates */
  _rate_estimator.print_rates(user_id, now);
}
