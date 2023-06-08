#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
#include <TLorentzVector.h>
#include <TChain.h>
#include <TMath.h>
#include <TFile.h>
#include <TH1F.h>
#include <vector>
#include <TH1F.h>
// #include <filesystem>
#include "yaml-cpp/yaml.h"

using namespace std;
using namespace ROOT;

TChain *init_chain(const char *file_path, const char *tree_name)
{
    TChain *chain = new TChain(tree_name);
    chain->Add(file_path);
    return chain;
}

auto ApplyDefines(RDF::RNode df, const std::vector<std::string> &names, const std::vector<std::string> &exprs, unsigned int i = 0)
{
    if (i == names.size())
        return df;
    return ApplyDefines(df.Define(names[i], exprs[i]), names, exprs, i + 1);
}

auto ApplyFilters(RDF::RNode df, const std::vector<std::string> &names, const std::vector<std::string> &exprs, unsigned int i = 0)
{
    if (i == names.size())
        return df;
    return ApplyFilters(df.Filter(exprs[i], names[i]), names, exprs, i + 1);
}

int main(int argc, char *argv[])
{
    // Parse command line arguments
    std::string config_file = argv[1];

    bool verbose = false;  // Default verbosity value
    bool make_histograms = true; // Default save histograms value

    if (argc > 2 && std::string(argv[2]) == "true") {
        verbose = true;  // Enable verbosity if second argument is "true"
    }

   if (argc > 3 && std::string(argv[3]) == "false") {
        make_histograms = false;  // Enable verbosity if second argument is "true"
    }

    /////////////////////
    // PARSE YAML FILE //

    // Load the YAML file
    YAML::Node config = YAML::LoadFile(config_file);

    // Extract variables
    const std::string process = config["process"].as<std::string>();

    // // Bins for differential distributions
    // std::vector<std::pair<float, float>> BINS;
    // for (const auto &bin : config["BINS"])
    // {
    //     float bin_start = bin[0].as<float>();
    //     float bin_end = bin[1].as<float>();
    //     BINS.push_back(std::make_pair(bin_start, bin_end));
    // }
    // int NBINS = BINS.size();

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

    std::vector<RDF::RResultPtr<RDF::RCutFlowReport>> cutFlowReports;
    std::vector<int> NCUTS;
    std::vector<int> allEntries;

    for (uint sample_idx = 0; sample_idx < sample_names.size(); sample_idx++)
    {
        // std::vector<TH1F> histograms;
        std::vector<ROOT::RDF::RResultPtr<TH1D>> histograms;

        // Read cuts for this sample
        std::vector<std::string> cut_names;
        std::vector<std::string> cut_exprs;
        int ncuts = 0;
        YAML::Node cuts = samples[sample_idx]["cuts"];
        for (const auto &cut : cuts)
        {
            cut_names.push_back(cut["name"].as<std::string>());
            cut_exprs.push_back(cut["expression"].as<std::string>());
            ncuts++;
        }

        // Get sample TChain
        TChain *chain = init_chain(sample_paths[sample_idx].c_str(), sample_trees[sample_idx].c_str());
        TFile *f_hist;
        if(config["histograms"] && make_histograms)
        {
            std::string configPath = argv[1];
            std::string baseName;

            // Find the last underscore in the config path
            size_t lastUnderscorePos = configPath.find_last_of('_');

            // Extract the base name
            if (lastUnderscorePos != std::string::npos) {
                baseName = configPath.substr(0, lastUnderscorePos);
            } else {
                // Handle the case where there is no underscore in the config path
                // You can choose an appropriate default value or error handling here
                baseName = "default";
            }
            // Find the last slash in the config path
            size_t lastSlashPos = baseName.find_last_of('/');

            // Extract the base name
            if (lastSlashPos != std::string::npos) {
                baseName = baseName.substr(lastSlashPos + 1);
            } else {
                // Handle the case where there is no slash in the config path
                baseName = baseName;
            }

            // Concatenate the strings
            cout << "baseName: " << baseName << endl;
            std::string outputFileName = "rootfiles/histograms_" + baseName + ".root";

            f_hist = new TFile(outputFileName.c_str(), "RECREATE");
        }

        // Create RDataFrame
        auto df = RDataFrame(*chain);
        allEntries.push_back(df.Count().GetValue());
        auto df_withCols = ApplyDefines(df, def_names, def_exprs);
        auto df_withCuts = ApplyFilters(df_withCols, cut_names, cut_exprs);
        auto cutFlowReport = df_withCuts.Report();
        cutFlowReports.push_back(cutFlowReport);

        cout << "Number of cuts: " << ncuts << endl;
        NCUTS.push_back(ncuts);

        // Loop over all the cuts
        int idx = -1;
        for (auto &&cutFlowReport : cutFlowReports)
        {
            idx++;

            // Create a copy of df_withCols for histograms
            auto df_forHistos = df_withCols;

            // Create histograms before any cut
            if(config["histograms"] && make_histograms)
            {
                for (const auto &histogram : config["histograms"])
                {
                    std::string name = histogram["name"].as<std::string>();
                    std::string variable = histogram["variable"].as<std::string>();
                    int nbins = histogram["nbins"].as<int>();
                    float xmin = histogram["xmin"].as<float>();
                    float xmax = histogram["xmax"].as<float>();

                    std::string hist_name = name + "_noCuts";
                    auto hist = df_forHistos.Histo1D({hist_name.c_str(), hist_name.c_str(), nbins, xmin, xmax}, variable.c_str());
                    histograms.push_back(hist);
                    if(verbose) cout << "Pushed back histogram " << hist_name << endl;
                }

                // Apply cuts and create histograms
                for (int i = 0; i < NCUTS[idx]; i++)
                {
                    std::string cut_name = cut_names[i];
                    std::string expression = cut_exprs[i];
                    df_forHistos = df_forHistos.Filter(expression.c_str());

                    for (const auto &histogram : config["histograms"])
                    {
                        std::string name = histogram["name"].as<std::string>();
                        std::string variable = histogram["variable"].as<std::string>();
                        int nbins = histogram["nbins"].as<int>();
                        float xmin = histogram["xmin"].as<float>();
                        float xmax = histogram["xmax"].as<float>();

                        std::string hist_name = name + "_" + cut_name;
                        auto hist = df_forHistos.Histo1D({hist_name.c_str(), hist_name.c_str(), nbins, xmin, xmax}, variable.c_str());
                        histograms.push_back(hist);
                        if(verbose) cout << "Pushed back histogram " << hist_name << endl;
                    }
                }
            }
        }

        // Write histograms to histograms.root
        if(config["histograms"] && make_histograms)
        {
            f_hist->cd();
            if(verbose) cout << "Writing histograms to histograms.root" << endl;
            for (auto &histogram : histograms)
            {
                TH1D* hist = histogram.GetPtr();
                if(verbose) cout << "Writing " << hist->GetName() << endl;
                hist->Write();
            }
            f_hist->Close();

            delete chain;
            delete f_hist;
        }
    }

    int idx = -1;
    for (auto &&cutFlowReport : cutFlowReports)
    {
        idx++;

        double allEntries = -1;

        cout << process << "_" << sample_names[idx] << endl;

        std::cout << std::left << std::setw(20) << "Name"
                               << std::setw(20) << "All"
                               << std::setw(20) << "Pass"
                               << std::setw(20) << "Efficiency %"
                               << std::setw(20) << "Uncertainty %"
                               << std::setw(20) << "Tot. Efficiency %"
                               << std::setw(20) << "Tot. Uncertainty %" << std::endl;

        int cut_idx = -1;
        for (auto &&cutInfo : cutFlowReport)
        {
            cut_idx++;

            if (idx > 0)
            {
                if (cut_idx < NCUTS[idx - 1])
                {
                    continue;
                }
            }

            double p = cutInfo.GetEff() / 100.;
            double N = cutInfo.GetAll();

            if (allEntries < 0)
            {
                allEntries = N;
            }

            double delta = TMath::Sqrt(p * (1 - p) / N);

            double tot_eff = cutInfo.GetPass() / allEntries;
            double tot_unc = TMath::Sqrt(tot_eff * (1 - tot_eff) / allEntries);

            std::cout << std::left << std::setw(20) << cutInfo.GetName()
                                   << std::setw(20) << cutInfo.GetAll()
                                   << std::setw(20) << cutInfo.GetPass()
                                   << std::setw(20) << cutInfo.GetEff()
                                   << std::setw(20) << delta * 100
                                   << std::setw(20) << tot_eff * 100
                                   << std::setw(20) << tot_unc * 100 << std::endl;
        }

        cout << endl;
    }

    return 0;
}