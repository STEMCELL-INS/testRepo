#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED



# include "base.h"
# include "data.h"
# include "circle.h"

void RandomNormalPair( reals& x, reals& y );

void SimulateArc(Data& data, reals a, reals b, reals R, reals theta1, reals theta2, reals sigma);

void SimulateRandom(Data& data, reals Window);

reals Sigma (Data& data, Circle& circle);

reals SigmaReduced (Data& data, Circle& circle);

reals SigmaReducedNearLinearCase (Data& data, Circle& circle);

reals SigmaReducedForCenteredScaled (Data& data, Circle& circle);

reals OptimalRadius (Data& data, Circle& circle);

# endif