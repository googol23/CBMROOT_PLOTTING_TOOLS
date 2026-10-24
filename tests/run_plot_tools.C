void run_plot_tools()
{
  const char* vmc_workdir   = gSystem->Getenv("VMCWORKDIR");
  const char* simpath       = gSystem->Getenv("SIMPATH");
  const char* fairroot_path = gSystem->Getenv("FAIRROOTPATH");
  if (!vmc_workdir || !simpath || !fairroot_path) {
    Error("run_plot_tools", "Source the CBMROOT config.sh before running this suite");
    gSystem->Exit(2);
    return;
  }

  gSystem->AddIncludePath(Form("-I%s/../install/include -I%s/include -I%s/include/fairlogger -I%s/include", vmc_workdir,
                               simpath, simpath, fairroot_path));
  if (gROOT->LoadMacro("tests/test_plot_tools.C+") < 0) {
    gSystem->Exit(1);
    return;
  }
  gROOT->ProcessLine("test_plot_tools();");
}
