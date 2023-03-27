/*
Copyright 2022 The Foedag team

GPL License

Copyright (c) 2022 The Open-Source FPGA Foundation

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BitAssembler.h"

#include "BitAssembler_mgr.h"
#include "BitGenerator/BitGenerator.h"
#include "CFGCommon/CFGArg_auto.h"
#include "CFGObject/CFGObject_auto.h"
#include "Utils/FileUtils.h"

void BitAssembler_entry(const CFGCommon_ARG* cmdarg) {
  CFG_TIME time_begin = CFG_time_begin();
  std::string bitasm_time = CFG_get_time();
  CFG_POST_MSG("This is BITASM entry");
  CFG_POST_MSG("  Project: %s", cmdarg->projectName.c_str());
  CFG_POST_MSG("  Device: %s", cmdarg->device.c_str());
  CFG_POST_MSG("  Time: %s", bitasm_time.c_str());
  std::string bitasm_file = CFG_print(
      "%s/%s.bitasm", cmdarg->projectPath.c_str(), cmdarg->projectName.c_str());
  std::string cfgbit_file = CFG_print(
      "%s/%s.cfgbit", cmdarg->projectPath.c_str(), cmdarg->projectName.c_str());
  CFG_POST_MSG("  Operation: %s", (cmdarg->clean ? "clean" : "generate"));
  bool bitgen = false;
  if (cmdarg->clean) {
    FOEDAG::FileUtils::removeFile(bitasm_file);
    FOEDAG::FileUtils::removeFile(cfgbit_file);
  } else {
    CFG_POST_MSG("  Output: %s", bitasm_file.c_str());
    CFGObject_BITOBJ bitobj;
    bitobj.write_str("version", "Raptor 1.0");
    bitobj.write_str("project", cmdarg->projectName);
    bitobj.write_str("device", cmdarg->device);
    bitobj.write_str("time", bitasm_time);

    // FCB
    BitAssembler_MGR mgr(cmdarg->projectPath, cmdarg->device);
    mgr.get_fcb(&bitobj.fcb);

    // Writing out
    bitgen = bitobj.write(bitasm_file);
    CFG_POST_MSG("  Status: %s", bitgen ? "success" : "fail");
  }
  CFG_POST_MSG("BITASM elapsed time: %.3f seconds",
               CFG_time_elapse(time_begin));
  if (bitgen) {
    CFGCommon_ARG* cmdarg_ptr = const_cast<CFGCommon_ARG*>(cmdarg);
    CFGArg_BITGEN bitgen_arg;
    bitgen_arg.Args.push_back(bitasm_file);
    bitgen_arg.Args.push_back(cfgbit_file);
    cmdarg_ptr->arg = &bitgen_arg;
    BitGenerator_entry(cmdarg_ptr);
  }
}
