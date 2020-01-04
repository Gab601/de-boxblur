#include <iostream>

#include "a10.h"

using namespace std;

void testDeblurX(const Image &im, string im_name, int radius) {
    Image out = deBoxBlurX(im, radius, true);
    out.write("./Output/" + im_name + "_dbx.png");
}

void testDeblurY(const Image &im, string im_name, int radius) {
    Image out = deBoxBlurY(im, radius, true);
    out.write("./Output/" + im_name + "_dby.png");
}

void testDeblur(const Image &im, string im_name, int radius) {
    Image out = deBoxBlur(im, radius, true);
    out.write("./Output/" + im_name + "_db.png");
}


int main()
{
    // Test your intermediate functions
    std::vector<string> im_names;
    im_names.push_back("forest");
    //im_names.push_back("landscape_small");
    //im_names.push_back("paper");
    //im_names.push_back("chairs");
    //im_names.push_back("outside");
    for (string im_name: im_names) {
      int radius = 3;
      Image im("./In/" + im_name + ".png");
      im = boxBlur(im, radius);
      for (int c = 0; c < im.channels(); c++) {
        for (int y = 0; y < im.height(); y++) {
          for (int x = 0; x < im.width(); x++) {
            im(x, y, c) = im(x, y, c) + float(rand() % 100) / 1000.0f;
          }
        }
      }
      im.write("./Output/" + im_name + "_blurred.png");
      testDeblurX(im, im_name, radius);
      testDeblurY(im, im_name, radius);
      testDeblur(im, im_name, radius);
    }
    return EXIT_SUCCESS;
}
