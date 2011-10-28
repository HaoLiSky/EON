//-----------------------------------------------------------------------------------
// eOn is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// A copy of the GNU General Public License is available at
// http://www.gnu.org/licenses/
//-----------------------------------------------------------------------------------

#include <math.h>
#include "Prefactor.h"
#include "Hessian.h"

int getPrefactors(Parameters* parameters, Matter *min1, Matter *saddle, Matter *min2, double &pref1, double &pref2)
{
    VectorXd min1Freqs, saddleFreqs, min2Freqs;

    // determine which atoms moved in the process
    VectorXi atoms;
    atoms = movedAtoms(parameters, min1, saddle, min2);
    // cout<<"Hessian size: "<<size<<endl;
    int size = 3*atoms.rows();
    assert(size > 0);

    // calculate min1 frequencies
    Hessian hessian(parameters, min1);
    min1Freqs = hessian.getFreqs(min1, atoms);
    if(min1Freqs.size() == 0)
    {
        if(!parameters->quiet) {
            printf("Bad hessian: min1\n");
        }
         return -1;
    }
    // remove zero modes
    if(parameters->checkRotation){
        hessian.removeZeroFreqs(min1Freqs);
    }

    // calculate saddle frequencies
    saddleFreqs = hessian.getFreqs(saddle, atoms);
    if(saddleFreqs.size() == 0)
    {
        if(!parameters->quiet) {
            printf("Bad hessian: saddle\n");
        }
        return -1;
    }
    // remove zero modes
    if(parameters->checkRotation){
        hessian.removeZeroFreqs(saddleFreqs);
    }

    // calculate min2 frequencies
    min2Freqs = hessian.getFreqs(min2, atoms);
    if(min2Freqs.size() == 0)
    {
        if(!parameters->quiet) {
            printf("Bad hessian: min2\n");
        }
        return -1;
    }
    // remove zero modes
    if(parameters->checkRotation){
        hessian.removeZeroFreqs(min2Freqs);
    }

    // check Hessian sizes
    if((min1Freqs.size() != saddleFreqs.size()) || (min1Freqs.size() != saddleFreqs.size())) {
        if(!parameters->quiet) {
            printf("Bad prefactor: Hessian sizes do not match\n");
        }
        return -1;
    }

    // check for correct number of negative modes

    int i, numNegFreq = 0;
    for(i=0; i<size; i++)
    {
        if(min1Freqs(i) < 0) { numNegFreq++; }
    }
    if(numNegFreq != 0)
    {
        cout<<"Error: "<<numNegFreq<<" negative modes at min1"<<endl;
        return -1;
    }

    numNegFreq = 0;
    for(i=0; i<size; i++)
    {
        if(saddleFreqs(i) < 0) { numNegFreq++; }
    }
    if(numNegFreq != 1)
    {
        cout<<"Error: "<<numNegFreq<<" negative modes at the saddle"<<endl;
        return -1;
    }

    numNegFreq = 0;
    for(i=0; i<size; i++)
    {
        if(min1Freqs(i) < 0) { numNegFreq++; }
    }
    if(numNegFreq != 0)
    {
        cout<<"Error: "<<numNegFreq<<" negative modes at min2"<<endl;
        return -1;
    }

    // calculate the prefactors
    pref1 = 1.0;
    pref2 = 1.0;

    // products are calculated this way in order to avoid overflow
    for(int i=0; i<saddleFreqs.size(); i++)
    {
        pref1 *= min1Freqs[i];
        pref2 *= min2Freqs[i];
        if(saddleFreqs[i]>0)
        {
            pref1 /= saddleFreqs[i];
            pref2 /= saddleFreqs[i];
        }
    }
    pref1 = sqrt(pref1)/(2*M_PI*10.18e-15);
    pref2 = sqrt(pref2)/(2*M_PI*10.18e-15);

    return 0;
}

VectorXi movedAtoms(Parameters* parameters, Matter *min1, Matter *saddle, Matter *min2)
{
    long nAtoms = saddle->numberOfAtoms();

    VectorXi moved(nAtoms);
    moved.setConstant(-1);

    AtomMatrix diffMin1 = saddle->pbc(saddle->getPositions() - min1->getPositions());
    AtomMatrix diffMin2 = saddle->pbc(saddle->getPositions() - min2->getPositions());

    diffMin1.cwise() *= saddle->getFree();
    diffMin2.cwise() *= saddle->getFree();

    int nMoved = 0;
    for(int i=0; i<nAtoms; i++)
    {
        if( (diffMin1.row(i).norm() > parameters->prefactorMinDisplacement) || 
            (diffMin2.row(i).norm() > parameters->prefactorMinDisplacement) )
        {
            if(!(moved.cwise() == i).any())
            {
                moved[nMoved] = i;
                nMoved++;
            }
            for(int j=0; j<nAtoms; j++)
            {
                double diffRSaddle = saddle->distance(i,j);

                if(diffRSaddle<parameters->prefactorWithinRadius
                   && (!saddle->getFixed(j)))
                {
                    if(!(moved.cwise() == j).any())
                    {
                        moved[nMoved] = j;
                        nMoved++;
                    }
                }
            }
        }
    }
    return (VectorXi) moved.block(0,0,nMoved,1);
}


VectorXd removeZeroFreqs(Parameters *parameters, VectorXd freqs)
{
    int size = freqs.size();
    VectorXd newfreqs(size);
    int nremoved = 0;
    for(int i=0; i<size; i++)
    {
        if(abs(freqs(i)) > parameters->hessianZeroFreqValue)
        {
            newfreqs(i-nremoved) = freqs(i);
        }
        else
        {
            nremoved++;
        }
    }
    if(nremoved != 6)
    {
        cout<<"Error: Found "<<nremoved<<" trivial eigenmodes instead of 6."<<endl;
    }
    return newfreqs;
}
