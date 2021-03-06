#include <algorithm>
#include "link/ensemble-rate-generator.h"

static class EnsembleRateGeneratorClass : public TclClass {
 public :
  EnsembleRateGeneratorClass() : TclClass("EnsembleRateGenerator") {}
  TclObject* create(int argc, const char*const* argv) {
    return (new EnsembleRateGenerator(std::string(argv[4])));
  }
} class_fcfs;

EnsembleRateGenerator::EnsembleRateGenerator(std::string t_trace_file)
    : trace_file_(t_trace_file) {
  assert(trace_file_!="");
  link_rate_changes_ = std::queue<LinkRateEvent>();
  num_users_ = read_link_rate_trace();
  assert(num_users_>0);
  printf("EnsembleRateGenerator, num_users_ %u, trace_file_ %s \n",
         num_users_, trace_file_.c_str());
  user_links_ = std::vector<LinkDelay*>(num_users_);
}

int EnsembleRateGenerator::command(int argc, const char*const* argv) {
  if (argc == 2) {
    if ( strcmp(argv[1], "get_users_" ) == 0 ) {
      Tcl::instance().resultf("%u",num_users_);
      return TCL_OK;
    }
  }
  if (argc == 2) {
    if ( strcmp(argv[1], "activate-rate-generator" ) == 0 ) {
      init();
      return TCL_OK;
    }
  }
  if(argc == 4) {
    if(!strcmp(argv[1],"attach-link")) {
      LinkDelay* link = (LinkDelay*) TclObject::lookup(argv[2]);
      uint32_t user_id = atoi(argv[3]);
      assert(user_id<user_links_.size());
      user_links_.at( user_id ) = link;
      return TCL_OK;
    }
  }
  return TclObject::command( argc, argv );
}

void EnsembleRateGenerator::expire(Event* e) {
  assert(Scheduler::instance().clock() >= next_event_.timestamp);
  assert(user_links_.at(next_event_.user_id)!=nullptr);
  user_links_.at(next_event_.user_id)->set_bandwidth(next_event_.link_rate);
  schedule_next_event();
}

uint32_t EnsembleRateGenerator::read_link_rate_trace(void) {
  FILE* f = fopen(trace_file_.c_str(), "r");
  if (f == NULL) {
    perror("fopen");
    exit(1);
  }
  assert(link_rate_changes_.empty());
  std::vector<uint32_t> unique_user_list;
  std::vector<LinkRateEvent> link_rate_change_vector;

  /* Populate event vector from file */
  while(1) {
    double ts, rate;
    uint32_t user_id;
    int num_matched = fscanf(f, "%lf %u %lf\n", &ts, &user_id, &rate);
    if (num_matched != 3) {
      break;
    }
    link_rate_change_vector.push_back(LinkRateEvent(ts, user_id, rate));
    if (std::find(unique_user_list.begin(), unique_user_list.end(), user_id) == unique_user_list.end()) {
      unique_user_list.push_back(user_id);
    }
  }

  /* Sort link_rate_change_vector */
  std::sort(link_rate_change_vector.begin(), link_rate_change_vector.end(),
            [&] (const LinkRateEvent &e1, const LinkRateEvent &e2)
            { return e1.timestamp < e2.timestamp; });

  /* Write into queue */
  for (uint32_t i=0; i < link_rate_change_vector.size(); i++) {
    if (!link_rate_changes_.empty()) {
      assert(link_rate_change_vector.at(i).timestamp >= link_rate_changes_.back().timestamp);
    }
    link_rate_changes_.push(link_rate_change_vector.at(i));
  }

  fclose(f);
  return unique_user_list.size();
}

void EnsembleRateGenerator::schedule_next_event() {
  if (link_rate_changes_.empty()) {
    force_cancel();
    return;
  }
  next_event_ = link_rate_changes_.front();
  link_rate_changes_.pop();
  assert(next_event_.timestamp >= Scheduler::instance().clock());
  auto time_to_next_event = next_event_.timestamp - Scheduler::instance().clock();
  resched(time_to_next_event);
}

void EnsembleRateGenerator::init() {
  schedule_next_event();
}
