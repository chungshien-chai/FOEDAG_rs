#include "Compiler/CompilerRS.h"
#include "Main/CommandLine.h"
#include "Main/Foedag.h"
#include "Main/Settings.h"
#include "Main/ToolContext.h"
#include "Main/WidgetFactory.h"
#include "MainWindow/Session.h"
#include "MainWindow/main_window.h"
#include "Utils/FileUtils.h"

namespace RS {

#define Company "Rapid Silicon"
#define ToolName "Raptor Design Suite"
#define ExecutableName "raptor"

QWidget* mainWindowBuilder(FOEDAG::Session* session) {
  FOEDAG::MainWindow* mainW = new FOEDAG::MainWindow{session};
  auto info = mainW->Info();
  info.name = QString("%1 %2").arg(Company, ToolName);
  info.url.clear();
  mainW->Info(info);
  return mainW;
}

void registerAllCommands(QWidget* widget, FOEDAG::Session* session) {
  registerAllFoedagCommands(widget, session);
}

}  // namespace RS

FOEDAG::GUI_TYPE getGuiType(const bool& withQt, const bool& withQml) {
  if (!withQt) return FOEDAG::GUI_TYPE::GT_NONE;
  if (withQml)
    return FOEDAG::GUI_TYPE::GT_QML;
  else
    return FOEDAG::GUI_TYPE::GT_WIDGET;
}

int main(int argc, char** argv) {
  FOEDAG::CommandLine* cmd = new FOEDAG::CommandLine(argc, argv);
  cmd->processArgs();

  FOEDAG::GUI_TYPE guiType = getGuiType(cmd->WithQt(), cmd->WithQml());
  FOEDAG::ToolContext* context =
      new FOEDAG::ToolContext(ToolName, Company, ExecutableName);
  FOEDAG::Settings* settings = new FOEDAG::Settings();
  FOEDAG::Compiler* compiler = nullptr;
  FOEDAG::CompilerRS* opcompiler = nullptr;
  if (cmd->CompilerName() == "dummy") {
    compiler = new FOEDAG::Compiler();
  } else {
    opcompiler = new FOEDAG::CompilerRS();
    compiler = opcompiler;
    compiler->SetParserType(FOEDAG::Compiler::ParserType::Verific);
    FOEDAG::addTclArgFns("CompilerRs_Synthesis",
                         {FOEDAG::TclArgs_setRsSynthesisOptions,
                          FOEDAG::TclArgs_getRsSynthesisOptions});
  }
  FOEDAG::Foedag* foedag =
      new FOEDAG::Foedag(cmd, RS::mainWindowBuilder, RS::registerAllCommands,
                         compiler, settings, context);
  if (opcompiler) {
    std::filesystem::path binpath = foedag->Context()->BinaryPath();
    std::filesystem::path datapath = foedag->Context()->DataPath();
    std::filesystem::path analyzePath = binpath / "analyze";
    std::filesystem::path yosysPath = binpath / "yosys";
    std::filesystem::path vprPath = binpath / "vpr";
    std::filesystem::path openFpgaPath = binpath / "openfpga";
    std::filesystem::path pinConvPath = binpath / "pin_c";
    std::filesystem::path staPath = binpath / "sta";
    std::filesystem::path starsPath = binpath / "stars";
    std::filesystem::path bitstreamSettingPath =
        datapath / "etc" / "devices" / "gemini" / "bitstream_annotation.xml";
    std::filesystem::path simSettingPath =
        datapath / "etc" / "devices" / "gemini" / "fixed_sim_openfpga.xml";
    std::filesystem::path repackConstraintPath = datapath / "etc" / "devices" /
                                                 "gemini" /
                                                 "repack_design_constraint.xml";
    std::filesystem::path openOcdPath = binpath / "openocd";
    opcompiler->AnalyzeExecPath(analyzePath);
    opcompiler->YosysExecPath(yosysPath);
    opcompiler->VprExecPath(vprPath);
    opcompiler->StaExecPath(staPath);
    opcompiler->StarsExecPath(starsPath);
    opcompiler->OpenFpgaExecPath(openFpgaPath);
    opcompiler->OpenFpgaBitstreamSettingFile(bitstreamSettingPath);
    opcompiler->OpenFpgaSimSettingFile(simSettingPath);
    opcompiler->OpenFpgaRepackConstraintsFile(repackConstraintPath);
    opcompiler->PinConvExecPath(pinConvPath);
    opcompiler->ProgrammerToolExecPath(openOcdPath);
    opcompiler->GetSimulator()->SetSimulatorPath(
        FOEDAG::Simulator::SimulatorType::Verilator,
        (binpath / "HDL_simulator" / "verilator" / "bin").string());
    opcompiler->GetSimulator()->SetSimulatorPath(
        FOEDAG::Simulator::SimulatorType::GHDL,
        (binpath / "HDL_simulator" / "GHDL" / "bin").string());
    opcompiler->GetSimulator()->SetSimulatorPath(
        FOEDAG::Simulator::SimulatorType::Icarus,
        (binpath / "HDL_simulator" / "iverilog" / "bin").string());
    std::filesystem::path configFileSearchDir = datapath / "configuration";
    opcompiler->SetConfigFileSearchDirectory(configFileSearchDir);
  }
  return foedag->init(guiType);
}
