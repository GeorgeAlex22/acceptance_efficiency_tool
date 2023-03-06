#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
// #include <ROOT/RNode.hxx>
#include <TLorentzVector.h>
#include <TChain.h>
#include <TMath.h>
#include <TFile.h>
#include <TH1F.h>

#include "yaml-cpp/yaml.h"

using namespace std;
using namespace ROOT;

TChain *init_chain(const char *file_path, const char *tree_name)
{
    TChain *chain = new TChain(tree_name);
    chain->Add(file_path);
    return chain;
}

auto ApplyDefines(RDF::RNode df, const std::vector<std::string> &names, const std::vector<std::string> &exprs, // TFile *f,
                  unsigned int i = 0)
{

    if (i == names.size())
        return df;

    return ApplyDefines(df.Define(names[i], exprs[i]), names, exprs, i + 1);
}

auto ApplyFilters(RDF::RNode df, const std::vector<std::string> &names, const std::vector<std::string> &exprs,
                  unsigned int i = 0)
{
    if (i == names.size())
        return df;

    return ApplyFilters(df.Filter(exprs[i], names[i]), names, exprs, i + 1);
}

int main(int argc, char *argv[])
{
    // Parse command line arguments
    std::string config_file = argv[1];

    /////////////////////
    // PARSE YAML FILE

    // Load the YAML file
    YAML::Node config = YAML::LoadFile(config_file);

    // Extract variables
    // const bool verbose = config["verbose"].as<bool>();
    const std::string process = config["process"].as<std::string>();

    // Bins for differential distributions
    std::vector<std::pair<float, float>> BINS;
    for (const auto &bin : config["BINS"])
    {
        float bin_start = bin[0].as<float>();
        float bin_end = bin[1].as<float>();
        BINS.push_back(std::make_pair(bin_start, bin_end));
    }
    int NBINS = BINS.size();

    // Read samples
    std::vector<std::string> sample_names;
    std::vector<std::string> sample_trees;
    std::vector<std::string> sample_paths;

    YAML::Node samples = config["samples"];
    for (const auto &sample : samples)
    {
        sample_names.push_back(sample["name"].as<std::string>());
        sample_trees.push_back(sample["tree"].as<std::string>());
        sample_paths.push_back(sample["paths"].as<std::string>());
    }

    // Read column definitions
    std::vector<std::string> def_names;
    std::vector<std::string> def_exprs;

    YAML::Node definitions = config["definitions"];
    for (const auto &def : definitions)
    {
        def_names.push_back(def["name"].as<std::string>());
        def_exprs.push_back(def["expression"].as<std::string>());
    }

    for (uint sample_idx = 0; sample_idx < sample_names.size(); sample_idx++)
    {
        // int sample_idx = 0;

        // Read cuts for this sample
        std::vector<std::string> cut_names;
        std::vector<std::string> cut_exprs;

        YAML::Node cuts = samples[sample_idx]["cuts"];
        for (const auto &cut : cuts)
        {
            cut_names.push_back(cut["name"].as<std::string>());
            cut_exprs.push_back(cut["expression"].as<std::string>());
            // char hist_name[100];
            // sprintf(hist_name, "hist_q2_%s", sample_names[sample_idx].c_str());
            // TH1I *hist_q2 = new TH1I(hist_name, hist_name, ,);
        }

        // Get sample TChain
        TChain *chain = init_chain(sample_paths[sample_idx].c_str(), sample_trees[sample_idx].c_str());

        // TFile *f_hist = new TFile("histograms.root", "RECREATE");

        // ROOT::EnableImplicitMT(); // enable multi-threading

        // Create RDataFrame
        auto df = RDataFrame(*chain);

        int allEntries = df.Count().GetValue();

        auto df_withCols = ApplyDefines(df, def_names, def_exprs);

        auto df_withCuts = ApplyFilters(df_withCols, cut_names, cut_exprs);

        auto cutFlowReport = df_withCuts.Report();

        cout << endl;
        cout << process << "_" << sample_names[sample_idx] << endl;

        // cutFlowReport->Print();

        // We can now loop on the cuts
        std::cout << std::left << std::setw(20) << "Name" << std::setw(20) << "All" << std::setw(20) << "Pass"
                  << std::setw(20) << "Efficiency %" << std::setw(20) << "Uncertainty %"
                  << std::setw(20) << "Com. Efficiency %" << std::setw(20) << "Com. Uncertainty %" << std::endl;
        for (auto &&cutInfo : cutFlowReport)
        {
            double p = cutInfo.GetEff() / 100.;
            double N = cutInfo.GetAll();
            double delta = TMath::Sqrt(p * (1 - p) / N) * 100; // calculate the uncertainty as a percentage
            double com_eff = cutInfo.GetPass() / (double)allEntries;
            double com_unc = TMath::Sqrt(com_eff * (1 - com_eff) / (double)allEntries) * 100;
            std::cout << std::left << std::setw(20) << cutInfo.GetName() << std::setw(20) << cutInfo.GetAll()
                      << std::setw(20) << cutInfo.GetPass() << std::setw(20) << cutInfo.GetEff()
                      << std::setw(20) << delta << std::setw(20) << com_eff * 100 << std::setw(20) << com_unc * 100 << std::endl;
        }

        // cout << df_withCuts.Count().GetValue() << endl;
    }
    // cout << 1 << endl;
    // auto colnames = df.GetColumnNames();
    // for (auto colname : colnames) {
    //     std::cout << colname << "\n";
    // }

    // cout << 2 << endl;
    // YAML::Node cuts = config["cuts"];
    // for (const auto& cut : config["cuts"]) {
    //     std::string cut_name = cut["name"].as<std::string>();
    //     std::string cut_expression = cut["cut"].as<std::string>();
    //     std::cout << "Cut name: " << cut_name << "\n";
    //     std::cout << "Cut expression: " << cut_expression << "\n";
    //     df.Filter(cut_expression);
    //     for (const auto& hist : cut["hists"]) {
    //         std::string hist_name = hist["name"].as<std::string>();
    //         std::string hist_value = hist["value"].as<std::string>();
    //         int hist_nbins = hist["nbins"].as<int>();
    //         float hist_min = hist["min"].as<float>();
    //         float hist_max = hist["max"].as<float>();

    //         // if (hist["define"].IsDefined()) {
    //         //     std::string hist_define = hist["define"].as<std::string>();
    //         //     df_cut = df_cut.Define(hist_value.c_str(), hist_define.c_str());
    //         // }

    //         auto histogram = df.Histo1D({hist_name.c_str(), hist_name.c_str(), hist_nbins, hist_min, hist_max}, hist_value.c_str());

    //         std::cout << "Hist name: " << hist_name << "\n";
    //         std::cout << "Hist value: " << hist_value << "\n";

    //         f_hist->Write(hist_name.c_str());
    //     }
    // }
    // cout << 3 << endl;
    // // string pt_filter = "(genMu1_pt > 6) || (genMu2_pt > 6)";
    // // string eta_filter = "(genMu1_pt > 6 && abs(genMu1_eta) < 2.4) || (genMu2_pt > 6 && abs(genMu2_eta) < 2.4)";

    // // cout << df.Count().GetValue() << endl;

    // // auto hist_pt_0 = df.Histo1D({"pt_0", "pt_0", 100, 0, 100}, "genMu1_pt");
    // // auto hist_eta_0 = df.Histo1D({"eta_0", "eta_0", 100, -2.5, 2.5}, "genMu1_eta");

    // // auto df_pt = df.Filter(pt_filter);
    // // auto hist_pt_1 = df_pt.Histo1D({"pt_1", "pt_1", 100, 0, 100}, "genMu1_pt");

    // // auto df_eta = df.Filter(eta_filter);
    // // auto hist_eta_1 = df_eta.Histo1D({"eta_1", "eta_1", 100, -2.5, 2.5}, "genMu1_eta");

    // // cout << df_eta.Count().GetValue() << endl;

    // f_hist->Close();

    return 0;
}