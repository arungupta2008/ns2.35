#ifndef ns_sfd_h
#define ns_sfd_h

#include <float.h>
#include <string.h>
#include "ensemble-aware-queue.h"
#include <map>
#include <list>
#include <queue>
#include "rng.h"
#include "common/flow-stats.h"
#include "queue/sfd-dropper.h"


/*
 * Stochastic Fair Dropping : Variation of AFD
 * where drop rates for TCP are set in accordance
 * with the TCP loss equation.
 */

class SFD : public EnsembleAwareQueue {
  private :

    /* Tcl accessible SFD parameters */
    double  _K;        /* default : 200 ms */
    double  _headroom; /* default : 0.05 */
    int _iter;         /* random seed */
    int user_id;       /* unique user_id */

    /* Underlying FIFO */
    PacketQueue* _packet_queue;

    /* Random dropper */
    SfdDropper _dropper;

    /* Rate Estimator */
    FlowStats _rate_estimator;

  public :
    SFD();
    int command(int argc, const char*const* argv) override;

    /* print stats  */
    void print_stats( double now );

    /* inherited functions from queue */
    virtual void enque( Packet *p ) override;
    virtual Packet* deque() override;
    virtual bool empty() const override { return (_packet_queue->byteLength() == 0); }
    virtual double get_hol() const override { return (empty()) ? DBL_MAX : hdr_cmn::access(_packet_queue->head())->timestamp(); }
    virtual double get_arrival_rate() const override { return _rate_estimator._arr_est.get_rate(); }

};

#endif
