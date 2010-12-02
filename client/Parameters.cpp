//-----------------------------------------------------------------------------------
// eOn is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// A copy of the GNU General Public License is available at
// http://www.gnu.org/licenses/
//-----------------------------------------------------------------------------------

#include <errno.h>
#include <time.h>
#include "Parameters.h"
#include "Hessian.h"
#include "INIFile.h"
#include "HelperFunctions.h"

Parameters::Parameters(){

    // Default values

    jobType = PROCESS_SEARCH;
    randomSeed = -1;
    reactantStateTag = 0;
    potentialTag = 1;
    potentialNoTranslation = 0;
    getPrefactorsTag = 0;
    minimizeOnly = 0;
    minimizeBox = 0;
    maxDifferencePos = 0.1;

    // for finding Epicenters
    neighborCutoff = 3.3;

    //Debug Options
    saveStdout=false;

    // default parameter for relaxation   
    convergedRelax = 0.005;

    // default parameters for process search
    processSearchMinimizeFirst = 0;

    // default parameters for saddle point determination
    saddleTypePerturbation = 1;
    saddleRefine = false;
    saddleLowestEigenmodeDetermination = 1;
    saddleConverged = 0.025;
    saddleMaxJumpAttempts = 0;
    saddleMaxStepSize = 0.2;
    saddleMaxEnergy = 20.0;
    saddleNormPerturbation = 0.1;
    saddleWithinRadiusPerturbated = 4.0;
    saddleMaxSinglePerturbation = 0.1;
    saddleMaxIterations = 512;
    saddleMaxIterationsConcave = 256;
    saddlePerpendicularForceRatio = 0.0;

    // default parameters for Hessian determination
    hessianKind = 0;
    hessianMaxSize = 0;
    hessianMinDisplacement = 0.25;
    hessianWithinRadiusDisplaced = 5.0;
    hessianPrefactorMax = 10e20;
    hessianPrefactorMin = 10e8;

    // default parameters the dimer method
    dimerSeparation = 0.0001;
    dimerRotationAngle = 0.005;
    dimerWindowHigh = 1.0;
    dimerWindowLow = 0.1;
    dimerRotationsHigh = 8;
    dimerRotationsLow = 1;

    // default parameters used by the optimizers
    maximumIterations=512;
    cgCurvatureStep = 0.001;
    cgMaxMoveFullRelax = 0.2;
    qmTimeStep = 0.1;

    // default parameters used by parallel repica dynamics
    mdTimeStep = 0.1;
    mdTemperature = 300.0;
    mdSteps = 1000;
    mdDephaseSteps = 200;
    mdDephaseConstrain = 2;
    mdDephaseCheckType = 1;
    mdMaxMovedDist = 2.0;
    mdRefine = false;
    mdAutoStop = false;
    mdRefineAccuracy = 20;
    mdCheckFreq = 500;
    mdRelaxSteps = 500;

    // default parameters used by hyperdynamics
    bondBoost = false ;
    bondBoostDVMAX = 0.0;
    bondBoostQRR = 0.0001; // can not be set to 0
    bondBoostPRR = 0.95;
    bondBoostQcut = 3.0;
    bondBoostRMDS = 0;

    // default parameters used by thermostat
    thermoType = ANDERSEN;
    thermoAndersenAlpha = 0.2; // collision strength
    thermoAndersenTcol = 10; // collision frequency in unit of dt
    thermoNoseMass = 0;
    return;

    // Default parameters for the displacement sampling job
    displaceNSamples = 32;              // The number of samples to take.
    displaceIterMax = 32;               // The maximum number of rotations to perform on the dimer.
    displaceTorqueConvergence = 0.01;   // The convergence criteria of the dimer rotation.
    displaceMaxCurvature = -0.1;        // The maximum curvature for which a sample is considered good. Used to avoid shallow but negative curvatures.
    displaceMaxDE = 10.0;               // The maximum dE for which a sample is considered good. XXX: Should use saddleMaxEnergy?
    displaceCutoffs = "0.0 3.3";
    displaceMagnitudes = "0.0625 0.125 0.25";
}

Parameters::~Parameters(){
    return;
}

string Parameters::toLowerCase(string s)
{
    for (string::size_type i = 0; i < s.length(); ++i) {
      s[i] = tolower(s[i]);
    }
    return s;
}

int Parameters::load(string filename)
{
    FILE *fh;

    fh = fopen(filename.c_str(), "rb");
    if (fh == NULL) {
        return 1;
    }
    int error = load(fh);
    fclose(fh);
    return error;
}

int Parameters::load(FILE *file){
    
    CIniFile ini;
    ini.CaseInsensitive();
    int error=0;
    printf("List of non-default parameters: ");
    if(ini.ReadFile(file))
    {
        printf("\n\n");
        // if we succesfully read the file, then parse it as an INI

        randomSeed = ini.GetValueL("Default", "RANDOM_SEED", randomSeed);
        // Initialize random generator
        if(randomSeed < 0){
            unsigned i = time(NULL);
            randomSeed = i;
            helper_functions::random(i);
        }else{
            helper_functions::random(randomSeed);
        }
        printf("Random seed is: %ld\n", randomSeed);

        potentialTag = ini.GetValueL("Default", "POTENTIAL_TAG", potentialTag);
        potentialNoTranslation = ini.GetValueL("Default", "POTENTIAL_NO_TRANSLATION", potentialNoTranslation);
        minimizeOnly = ini.GetValueL("Default", "MINIMIZE_ONLY", minimizeOnly);
        minimizeBox = ini.GetValueL("Default", "MINIMIZE_BOX", minimizeBox);
        convergedRelax = ini.GetValueF("Default", "CONVERGED_RELAX", convergedRelax);
        maximumIterations = ini.GetValueL("Default", "MAXIMUM_ITERATIONS", maximumIterations);
        maxDifferencePos = ini.GetValueF("Default", "MAX_DIFFERENCE_POS", maxDifferencePos);
        neighborCutoff = ini.GetValueF("Default", "NEIGHBOR_CUTOFF", neighborCutoff);

        string jobTypeString;
        jobTypeString = ini.GetValue("Default", "JOB_TYPE", "processsearch");
        jobTypeString = toLowerCase(jobTypeString);
        if (jobTypeString == "processsearch") {
            jobType = PROCESS_SEARCH;
        }else if (jobTypeString == "saddlesearch") {
            jobType = SADDLE_SEARCH;
        }else if (jobTypeString == "minimization") {
            jobType = MINIMIZATION;
        }else if (jobTypeString == "hessian") {
            jobType = HESSIAN;
        }else if (jobTypeString == "parallelreplica"){
            jobType = PARALLEL_REPLICA;
        }else if (jobTypeString == "replicaexchange"){
            jobType = REPLICA_EXCHANGE;         
        }else if (jobTypeString == "dimerdr"){
            jobType = DIMER_DR;
        }else if (jobTypeString == "dimerrotation"){
            jobType = DIMER_ROTATION;
        }else if (jobTypeString == "displacementsampling"){
            jobType = DISPLACEMENT_SAMPLING;
        }else if (jobTypeString == "test"){
            jobType = TEST;
        }
        else{
            fprintf(stderr, "Unknown JOB_TYPE: %s\n", jobTypeString.c_str());
            error = 1;
        }

        processSearchMinimizeFirst = ini.GetValueL("ProcessSearch", "minimize_first", processSearchMinimizeFirst);
        saveStdout= ini.GetValueB("Debug", "save_stdout", saveStdout);

        saddleTypePerturbation = ini.GetValueL("Saddle_Point", "TYPE_PERTURBATION", saddleTypePerturbation);
        saddleLowestEigenmodeDetermination = ini.GetValueL("Saddle_Point", "LOWEST_EIGENMODE_DETERMINATION", saddleLowestEigenmodeDetermination);
        saddleRefine = ini.GetValueB("Saddle_Point", "REFINE", saddleRefine); 
        saddleConverged = ini.GetValueF("Saddle_Point", "CONVERGED", saddleConverged);
        saddleMaxJumpAttempts = ini.GetValueL("Saddle_Point", "MAX_JUMP_ATTEMPTS", saddleMaxJumpAttempts);
        saddleMaxStepSize = ini.GetValueF("Saddle_Point", "MAX_STEP_SIZE", saddleMaxStepSize);
        saddleMaxEnergy = ini.GetValueF("Saddle_Point", "MAX_ENERGY", saddleMaxEnergy);
        saddleNormPerturbation = ini.GetValueF("Saddle_Point", "NORM_PERTURBATION", saddleNormPerturbation);
        saddleMaxSinglePerturbation = ini.GetValueF("Saddle_Point", "MAX_SINGLE_PERTURBATION", saddleMaxSinglePerturbation);
        saddleWithinRadiusPerturbated = ini.GetValueF("Saddle_Point", "WITHIN_RADIUS_PERTURBATED", saddleWithinRadiusPerturbated);
        saddleMaxIterations = ini.GetValueL("Saddle_Point", "MAX_ITERATIONS", saddleMaxIterations);
        saddleMaxIterations = ini.GetValueL("Saddle_Point", "MAX_ITERATIONS_CONCAVE", saddleMaxIterationsConcave);
        saddlePerpendicularForceRatio = ini.GetValueF("Default", "PERPENDICULAR_FORCE_RATIO", saddlePerpendicularForceRatio);

        string hessianType = ini.GetValue("Hessian", "Type", "reactant");
        hessianType = toLowerCase(hessianType);
        if(hessianType == "reactant"){
            hessianKind = Hessian::REACTANT;
        }else if(hessianType == "saddle"){
            hessianKind = Hessian::SADDLE;
        }else if(hessianType == "product"){
            hessianKind = Hessian::PRODUCT;
        }
        hessianMaxSize = ini.GetValueL("Hessian", "MAX_SIZE", hessianMaxSize);
        hessianWithinRadiusDisplaced = ini.GetValueF("Hessian", "WITHIN_RADIUS_DISPLACED", hessianWithinRadiusDisplaced);
        hessianMinDisplacement = ini.GetValueF("Hessian", "MIN_DISPLACEMENT", hessianMinDisplacement);
 
        dimerRotationsHigh = ini.GetValueL("Dimer", "ROTATIONS_HIGH", dimerRotationsHigh);
        dimerRotationsLow = ini.GetValueL("Dimer", "ROTATIONS_LOW", dimerRotationsLow);
        dimerWindowHigh = ini.GetValueF("Dimer", "WINDOW_HIGH", dimerWindowHigh);
        dimerWindowLow = ini.GetValueF("Dimer", "WINDOW_LOW", dimerWindowLow);
        dimerSeparation = ini.GetValueF("Dimer", "SEPARATION", dimerSeparation);
        dimerRotationAngle = ini.GetValueF("Dimer", "ANGLE", dimerRotationAngle);

        displaceNSamples = ini.GetValueL("DisplacementSampling", "NSAMPLES", displaceNSamples);
        displaceIterMax = ini.GetValueL("DisplacementSampling", "ITERMAX", displaceIterMax);
        displaceTorqueConvergence = ini.GetValueF("DisplacementSampling", "TORQUE_CONVERGENCE", displaceTorqueConvergence);
        displaceMaxCurvature = ini.GetValueF("DisplacementSampling", "MAX_CURVATURE", displaceMaxCurvature);
        displaceMaxDE = ini.GetValueF("DisplacementSampling", "MAX_DE", displaceMaxDE);
        displaceCutoffs = ini.GetValue("DisplacementSampling", "CUTOFFS", displaceCutoffs);
        displaceMagnitudes = ini.GetValue("DisplacementSampling", "MAGNITUDES", displaceMagnitudes);

        mdTimeStep = ini.GetValueF("Dynamics","TIMESTEP",mdTimeStep);
        mdTemperature = ini.GetValueF("Dynamics","TEMPERATURE",mdTemperature);
        mdSteps = ini.GetValueL("Dynamics","mdSTEPS",mdSteps);
        mdDephaseSteps = ini.GetValueL("Dynamics","Dephase_Steps",mdDephaseSteps);
        mdDephaseConstrain = ini.GetValueL("Dynamics","Dephase_Constrain",mdDephaseConstrain);
        mdDephaseCheckType = ini.GetValueL("Dynamics","Dephase_CheckType",mdDephaseCheckType);
        mdMaxMovedDist = ini.GetValueF("Dynamics","PRD_MaxMovedDist",mdMaxMovedDist);  
        mdRefine = ini.GetValueB("Dynamics","mdRefine",mdRefine);
        mdAutoStop = ini.GetValueB("Dynamics","mdAutoStop",mdAutoStop);
        mdRefineAccuracy = ini.GetValueL("Dynamics","RefineAccuracy",mdRefineAccuracy);
        mdCheckFreq = ini.GetValueL("Dynamics","CheckFreq", mdCheckFreq);
        mdRelaxSteps = ini.GetValueL("Dynamics","newRelaxStep",mdRelaxSteps);

        lanczosConvergence = ini.GetValueF("Lanczos", "CONVERGENCE", lanczosConvergence);
        lanczosIteration = ini.GetValueL("Lanczos", "ITERATION", lanczosIteration);

        bondBoost = ini.GetValueB("Hyper","BondBoost",bondBoost);
        bondBoostRMDS = ini.GetValueL("Hyper","RMDS",bondBoostRMDS);
        bondBoostDVMAX = ini.GetValueF("Hyper","DVMAX",bondBoostDVMAX);
        bondBoostQRR = ini.GetValueF("Hyper","QRR",bondBoostQRR );
        bondBoostPRR = ini.GetValueF("Hyper","PRR",bondBoostPRR );
        bondBoostQcut= ini.GetValueF("Hyper","Qcut",bondBoostQcut);

        cgCurvatureStep = ini.GetValueF("CG","CURVATURE_STEP", cgCurvatureStep);

        string thermoTypeString;
        thermoTypeString = ini.GetValue("Default", "THERMO_TYPE", "andersen");
        thermoTypeString = toLowerCase(thermoTypeString);
        if (thermoTypeString == "andersen") {
            thermoType = ANDERSEN;
        }else if (thermoTypeString == "nosehover") {
            thermoType = NOSE_HOVER;
        }
        thermoAndersenAlpha = ini.GetValueF("Thermo","ANDERSEN_ALPHA",thermoAndersenAlpha);
        thermoAndersenTcol = ini.GetValueF("Thermo","ANDERSEN_TCOL",thermoAndersenTcol);
        thermoNoseMass = ini.GetValueF("Thermo","NoseMass",thermoNoseMass);
    }
    else
    {
        fprintf(stderr, "Couldn't parse the ini file. Perhaps you are "
                        "using the old style config?\n");
        error = 1;
    }
    return error;
}
