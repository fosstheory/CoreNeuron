#include "report_event.h"
#include "coreneuron/nrnoc/multicore.h"
#include "coreneuron/nrniv/nrn_assert.h"
#ifdef ENABLE_REPORTING
#include "bbp/sonata/reports.h"
#include "reportinglib/Records.h"
#endif  // ENABLE_REPORTING

namespace coreneuron {

#ifdef ENABLE_REPORTING
ReportEvent::ReportEvent(double dt, double tstart, VarsToReport &filtered_gids, const char *name)
        : dt(dt), tstart(tstart) {
    strcpy(report_path, name);
    VarsToReport::iterator it;
    nrn_assert(filtered_gids.size());
    step = tstart / dt;
    gids_to_report.reserve(filtered_gids.size());
    for (it = filtered_gids.begin(); it != filtered_gids.end(); ++it) {
        gids_to_report.push_back(it->first);
    }
    std::sort(gids_to_report.begin(), gids_to_report.end());
}

/** on deliver, call ReportingLib and setup next event */
void ReportEvent::deliver(double t, NetCvode *nc, NrnThread *nt) {
/** @todo: reportinglib is not thread safe */
#pragma omp critical
    {
        // each thread needs to know its own step
        sonata_record_node_data(step, gids_to_report.size(), &gids_to_report[0], report_path);
        records_nrec(step, gids_to_report.size(), &gids_to_report[0], report_path);
        send(t + dt, nc, nt);
        step++;
    }
}

bool ReportEvent::require_checkpoint() {
    return false;
}
#endif  // ENABLE_REPORTING

} // Namespace coreneuron