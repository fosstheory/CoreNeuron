/*
Copyright (c) 2016, Blue Brain Project
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _H_NRNSETUP_
#define _H_NRNSETUP_

#include <string>
#include "coreneuron/sim/multicore.hpp"
#include "coreneuron/io/nrn_filehandler.hpp"

namespace coreneuron {
static bool do_not_open;
static int ngroup_w;
static int* gidgroups_w;
static int* imult_w;
static const char* path_w;
static const char* restore_path_w;
static FileHandler* file_reader_w;
static bool byte_swap_w;

static void read_phase1(FileHandler& F, int imult, NrnThread& nt);
static void read_phase2(FileHandler& F, int imult, NrnThread& nt);
static void read_phase3(FileHandler& F, int imult, NrnThread& nt);
static void read_phasegap(FileHandler& F, int imult, NrnThread& nt);
static void setup_ThreadData(NrnThread& nt);

// Functions to load and clean data;
extern void nrn_init_and_load_data(int argc,
                                   char** argv,
                                   bool is_mapping_needed = false,
                                   bool nrnmpi_under_nrncontrol = true,
                                   bool run_setup_cleanup = true);
extern void nrn_setup_cleanup();

extern int nrn_i_layout(int i, int cnt, int j, int size, int layout);

namespace coreneuron {

/// Reading phase number.
enum phase { one = 1, two, three, gap };

/// Get the phase number in form of the string.
template <phase P>
inline std::string getPhaseName();

template <>
inline std::string getPhaseName<one>() {
    return "1";
}

template <>
inline std::string getPhaseName<two>() {
    return "2";
}

template <>
inline std::string getPhaseName<three>() {
    return "3";
}

template <>
inline std::string getPhaseName<gap>() {
    return "gap";
}

/// Reading phase selector.
template <phase P>
inline void read_phase_aux(FileHandler& F, int imult, NrnThread& nt);

template <>
inline void read_phase_aux<one>(FileHandler& F, int imult, NrnThread& nt) {
    read_phase1(F, imult, nt);
}

template <>
inline void read_phase_aux<two>(FileHandler& F, int imult, NrnThread& nt) {
    read_phase2(F, imult, nt);
}

template <>
inline void read_phase_aux<three>(FileHandler& F, int imult, NrnThread& nt) {
    read_phase3(F, imult, nt);
}

template <>
inline void read_phase_aux<gap>(FileHandler& F, int imult, NrnThread& nt) {
    read_phasegap(F, imult, nt);
}

/// Reading phase wrapper for each neuron group.
template <phase P>
inline void* phase_wrapper_w(NrnThread* nt) {
    bool no_open = do_not_open;
    int i = nt->id;
    char fnamebuf[1000];
    if (i < ngroup_w) {
        if (!no_open) {
            const char* data_dir = path_w;
            // directory to read could be different for phase 2 if we are restoring
            // all other phases still read from dataset directory because the data
            // is constant
            if (P == 2) {
                data_dir = restore_path_w;
            }

            std::string fname = std::string(data_dir) + "/" + std::to_string(gidgroups_w[i]) + "_" + getPhaseName<P>() + ".dat";

            // Avoid trying to open the gid_gap.dat file if it doesn't exist when there are no
            // gap junctions in this gid
            if (P == gap && !file_reader_w[i].file_exist(fname.c_str())) {
                file_reader_w[i].close();
            } else {
                // if no file failed to open or not opened at all
                file_reader_w[i].open(fname.c_str(), byte_swap_w);
            }
        }
        read_phase_aux<P>(file_reader_w[i], imult_w[i], *nt);
        if (!no_open) {
            file_reader_w[i].close();
        }
        if (P == 2) {
            setup_ThreadData(*nt);
        }
    }
    return nullptr;
}

/// Specific phase reading executed by threads.
template <phase P>
inline static void phase_wrapper(int direct = 0) {
    if (direct) {
        do_not_open = true;
    }
    nrn_multithread_job(phase_wrapper_w<P>);
    if (direct) {
        do_not_open = false;
    }
}
}  // namespace coreneuron
}  // namespace coreneuron
#endif
