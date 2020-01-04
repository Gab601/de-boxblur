#include <iostream>
#include <algorithm>

#include "a10.h"

using namespace std;

// Write your implementations here, or extend the Makefile if you add source
// files
void someFunction() {
    cout << "ok, that's a function" << endl;
}

Image getDarkChannel(const Image &im, int radius) {
  Image out(im.width(), im.height(), 1);
  for (int y = 0; y < im.height(); y++) {
    for (int x = 0; x < im.width(); x++) {
      float min = im.smartAccessor(x, y, 0);
      for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
          for (int c = 0; c < im.channels(); c++) {
            if (im.smartAccessor(x+dx, y+dy, c) < min) {
              min = im.smartAccessor(x+dx, y+dy, c);
            }
          }
        }
      }
      out(x, y) = min;
    }
  }
  return out;
}

std::vector<float> getAtmosphericLight(const Image &im, int radius) {
  Image darkChannel = getDarkChannel(im, radius);
  Image bw = color2gray(im);
  std::vector<float> darkValues = getDarkChannel(im, radius).image_floats();
  sort(darkValues.begin(), darkValues.end());
  float cutoff = darkValues[.999*darkValues.size()];
  std::vector<std::vector<int>> pixels;
  for (int y = 0; y < im.height(); y++) {
    for (int x = 0; x < im.width(); x++) {
      if (darkChannel(x, y) >= cutoff) {
        std::vector<int> p;
        p.push_back(x);
        p.push_back(y);
        pixels.push_back(p);
      }
    }
  }
  float max_lumi = bw(pixels[0][0], pixels[0][1]);
  int x_coord = pixels[0][0];
  int y_coord = pixels[0][1];
  for (std::vector<int> p: pixels) {
    if (bw(p[0], p[1]) > max_lumi) {
      x_coord = p[0];
      y_coord = p[1];
    }
  }
  std::vector<float> color;
  for (int c = 0; c < im.channels(); c++) {
    color.push_back(im(x_coord, y_coord, c));
  }
  return color;
}

Image getTransmission(const Image &im, int radius, float omega) {
  Image lightAdjusted(im.width(), im.height(), im.channels());
  std::vector<float> atmosphericLight = getAtmosphericLight(im, radius);
  for (int y = 0; y < im.height(); y++) {
    for (int x = 0; x < im.width(); x++) {
      for (int c = 0; c < im.channels(); c++) {
        lightAdjusted(x, y, c) = im(x, y, c) / atmosphericLight[c];
      }
    }
  }
  Image transmission = 1 - omega * getDarkChannel(lightAdjusted, radius);
  return transmission;
}

Image guidedBilateralFilter(const Image &guide, const Image &transmission,
                            float sigmaRange,
                            float sigmaDomain,
                            float truncateDomain) {
    Image imFilter(transmission.width(), transmission.height(), transmission.channels());

    // calculate the filter size
    int offset   = int(ceil(truncateDomain * sigmaDomain));
    int sizeFilt = 2*offset + 1;
    float accum,
          tmp,
          range_dist,
          normalizer,
          factorDomain,
          factorRange;

    // for every pixel in the image
    for (int z=0; z<imFilter.channels(); z++)
    for (int y=0; y<imFilter.height(); y++)
    for (int x=0; x<imFilter.width(); x++)
    {
        // initilize normalizer and sum value to 0 for every pixel location
        normalizer = 0.0f;
        accum      = 0.0f;

        // sum over the filter's support
        for (int yFilter=0; yFilter<sizeFilt; yFilter++)
        for (int xFilter=0; xFilter<sizeFilt; xFilter++)
        {
            // calculate the distance between the 2 pixels (in range)
            range_dist = 0.0f; // |R-R1|^2 + |G-G1|^2 + |B-B1|^2
            for (int z1 = 0; z1 < imFilter.channels(); z1++) {
                tmp  = guide(x,y,z1); // center pixel
                tmp -= guide.smartAccessor(x+xFilter-offset,y+yFilter-offset,z1); // neighbor
                tmp *= tmp; // square
                range_dist += tmp;
            }

            // calculate the exponential weight from the domain and range
            factorDomain = exp( - ((xFilter-offset)*(xFilter-offset) +  (yFilter-offset)*(yFilter-offset) )/ (2.0 * sigmaDomain*sigmaDomain ) );
            factorRange  = exp( - range_dist / (2.0 * sigmaRange*sigmaRange) );

            normalizer += factorDomain * factorRange;
            accum += factorDomain * factorRange * transmission.smartAccessor(x+xFilter-offset,y+yFilter-offset,z);
        }

        // set pixel in filtered image to weighted sum of values in the filter region
        imFilter(x,y,z) = accum/normalizer;
    }

    return imFilter;
}

Image guidedBilateralFilter2(const Image &guide, const Image &transmission, const Image &factor,
                            float sigmaRange,
                            float sigmaDomain,
                            float truncateDomain) {
    Image imFilter(transmission.width(), transmission.height(), transmission.channels());

    // calculate the filter size
    int offset   = int(ceil(truncateDomain * sigmaDomain));
    int sizeFilt = 2*offset + 1;
    float accum,
          tmp,
          range_dist,
          normalizer,
          factorDomain,
          factorRange;

    // for every pixel in the image
    for (int z=0; z<imFilter.channels(); z++)
    for (int y=0; y<imFilter.height(); y++)
    for (int x=0; x<imFilter.width(); x++)
    {
        // initilize normalizer and sum value to 0 for every pixel location
        normalizer = 0.0f;
        accum      = 0.0f;

        // sum over the filter's support
        for (int yFilter=0; yFilter<sizeFilt; yFilter++)
        for (int xFilter=0; xFilter<sizeFilt; xFilter++)
        {
            // calculate the distance between the 2 pixels (in range)
            range_dist = 0.0f; // |R-R1|^2 + |G-G1|^2 + |B-B1|^2
            for (int z1 = 0; z1 < imFilter.channels(); z1++) {
                tmp  = guide(x,y,z1); // center pixel
                tmp -= guide.smartAccessor(x+xFilter-offset,y+yFilter-offset,z1); // neighbor
                tmp *= tmp; // square
                range_dist += tmp;
            }

            // calculate the exponential weight from the domain and range
            factorDomain = exp( - ((xFilter-offset)*(xFilter-offset) +  (yFilter-offset)*(yFilter-offset) )/ (2.0 * sigmaDomain*sigmaDomain ) );
            factorRange  = exp( - range_dist / (2.0 * sigmaRange*sigmaRange) );

            normalizer += factorDomain * factorRange * factor.smartAccessor(x+xFilter-offset, y+yFilter-offset, z);
            accum += factorDomain * factorRange * transmission.smartAccessor(x+xFilter-offset,y+yFilter-offset,z) * factor.smartAccessor(x+xFilter-offset, y+yFilter-offset, z);
        }

        // set pixel in filtered image to weighted sum of values in the filter region
        imFilter(x,y,z) = accum/normalizer;
    }

    return imFilter;
}

Image doubleGuidedBilateralFilter(const Image &guide, const Image &transmission,
                            float sigmaRange,
                            float sigmaDomain,
                            float truncateDomain) {
    Image imFilter1(transmission.width(), transmission.height(), transmission.channels());
    Image imFilter2(transmission.width(), transmission.height(), transmission.channels());

    imFilter1 = guidedBilateralFilter(guide, transmission, sigmaRange, sigmaDomain, truncateDomain);
    for (int z=0; z<imFilter1.channels(); z++)
    for (int y=0; y<imFilter1.height(); y++)
    for (int x=0; x<imFilter1.width(); x++)
    {
      imFilter1(x, y, z) = max(0.001f, min(1.0f, 1 + 100*(imFilter1(x, y, z) - transmission(x, y, z))));
    }
    imFilter1.write("./Out/filter_weights.png");
    imFilter2 = guidedBilateralFilter2(guide, transmission, imFilter1, sigmaRange, sigmaDomain*2, truncateDomain);
    return imFilter2;
}

Image dehaze(const Image &im, int radius, float omega) {
  std::vector<float> atmosphericLight = getAtmosphericLight(im, radius);
  Image transmission = guidedBilateralFilter(im, getTransmission(im, radius, omega));
  Image output(im.width(), im.height(), im.channels());
  for (int y = 0; y < im.height(); y++) {
    for (int x = 0; x < im.width(); x++) {
      for (int c = 0; c < im.channels(); c++) {
        output(x, y, c) = (im(x, y, c) - atmosphericLight[c])/max(transmission(x, y), 0.05f) + atmosphericLight[c];
      }
    }
  }
  return output;
}

Image dehaze_2(const Image &im, int radius, float omega) {
  std::vector<float> atmosphericLight = getAtmosphericLight(im, radius);
  Image transmission = doubleGuidedBilateralFilter(im, getTransmission(im, radius, omega));
  Image output(im.width(), im.height(), im.channels());
  for (int y = 0; y < im.height(); y++) {
    for (int x = 0; x < im.width(); x++) {
      for (int c = 0; c < im.channels(); c++) {
        output(x, y, c) = (im(x, y, c) - atmosphericLight[c])/max(transmission(x, y), 0.05f) + atmosphericLight[c];
      }
    }
  }
  return output;
}
