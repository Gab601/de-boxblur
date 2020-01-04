#ifndef A10_H_PHUDVTKB
#define A10_H_PHUDVTKB

#include "basicImageManipulation.h"
#include "Image.h"
#include "filtering.h"

// Write your declarations here, or extend the Makefile if you add source
// files
void someFunction();

Image getDarkChannel(const Image &im, int radius);

std::vector<float> getAtmosphericLight(const Image &im, int radius);

Image getTransmission(const Image &im, int radius, float omega);

Image guidedBilateralFilter(const Image &guide, const Image &transmission,
                float sigmaRange = 0.1,
                float sigmaDomain = 3.0,
                float truncateDomain = 2.0);

Image doubleGuidedBilateralFilter(const Image &guide, const Image &transmission,
                float sigmaRange = 0.1,
                float sigmaDomain = 3.0,
                float truncateDomain = 2.0);

Image guidedBilateralFilter2(const Image &guide, const Image &transmission, const Image &factor,
                            float sigmaRange = 0.1,
                            float sigmaDomain = 3.0,
                            float truncateDomain = 2.0);

Image dehaze(const Image &im, int radius, float omega);

Image dehaze_2(const Image &im, int radius, float omega);

#endif /* end of include guard: A10_H_PHUDVTKB */
