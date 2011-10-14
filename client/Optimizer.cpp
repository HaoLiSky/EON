#include "Optimizer.h"
#include "ConjugateGradients.h"
#include "Quickmin.h"
#include "LBFGS.h"

Optimizer *Optimizer::getOptimizer(ObjectiveFunction *objf, Parameters *parameters)
{
    Optimizer* mizer=NULL;
    if (parameters->optMethod == "cg") {
        mizer = new ConjugateGradients(objf, parameters);
    }else if (parameters->optMethod == "qm") {
        mizer = new Quickmin(objf, parameters);
    }else if (parameters->optMethod == "lbfgs") {
        mizer = new LBFGS(objf, parameters);
    }else{
        printf("Unknown optMethod: %s\n", parameters->optMethod.c_str());
        exit(1);
    }
    return mizer;
}