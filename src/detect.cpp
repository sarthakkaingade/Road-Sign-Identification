/* HOG DETECTOR
 *
 */

#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_transforms.h>
#include <dlib/data_io.h>
#include <dlib/cmd_line_parser.h>


#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using namespace dlib;

struct TrafficSign {
  string name;
  string svm_path;
  rgb_pixel color;
  TrafficSign(string name, string svm_path, rgb_pixel color) :
    name(name), svm_path(svm_path), color(color) {};
};

typedef scan_fhog_pyramid<pyramid_down<6> > image_scanner_type;

int process_video(const std::string video_name, const unsigned long upsample_amount, std::vector<TrafficSign> signs, std::vector<object_detector<image_scanner_type> > detectors)
{
    cv::VideoCapture cap(video_name);
    if (!cap.isOpened())
    {
        cout << "!!! Failed to open file: " << video_name << endl;
        return -1;
    }

    image_window win;
    while(1)  {
        cv::Mat frame;
        if (!cap.read(frame))    {
            cout << "Video sequence completed" << endl;
            break;
        }
        //cv::namedWindow( "Display window", CV_WINDOW_AUTOSIZE );
        //cv::imshow( "Display window", frame );
        cv_image<bgr_pixel> img(frame);
        for (unsigned long i = 0; i < upsample_amount; ++i) {
            //pyramid_up(img);
        }
        std::vector<rect_detection> rects;
        evaluate_detectors(detectors, img, rects);
        // Put the image and detections into the window.
        win.clear_overlay();
        win.set_image(img);
        for (unsigned long j = 0; j < rects.size(); ++j) {
            win.add_overlay(rects[j].rect, signs[rects[j].weight_index].color,signs[rects[j].weight_index].name);
            //sleep((unsigned int)1);
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    try {
        command_line_parser parser;

        parser.add_option("h","Display this help message.");
        parser.add_option("u", "Upsample each input image <arg> times. Each upsampling quadruples the number of pixels in the image (default: 0).", 1);
        parser.add_option("wait","Wait user input to show next image.");
        parser.add_option("video","Input Video to process.",1);

        parser.parse(argc, argv);
        parser.check_option_arg_range("u", 0, 8);

        const char* one_time_opts[] = {"h","u","wait"};
        parser.check_one_time_options(one_time_opts);

        // Display help message
        if (parser.option("h")) {
            cout << "Usage: " << argv[0] << " [options] <list of images>" << endl;
            parser.print_options();

            return EXIT_SUCCESS;
        }

        if (parser.number_of_arguments() == 0) {
            cout << "You must give a list of input files." << endl;
            cout << "\nTry the -h option for more information." << endl;
            return EXIT_FAILURE;
        }

        const unsigned long upsample_amount = get_option(parser, "u", 0);

        // Load SVM detectors
        std::vector<TrafficSign> signs;
        signs.push_back(TrafficSign("PEDESTRIAN", "trained_detectors/pedestrian_detector.svm",rgb_pixel(255,0,0)));
        signs.push_back(TrafficSign("SPEED-LIMIT", "trained_detectors/speed_detector.svm",rgb_pixel(255,122,0)));
        signs.push_back(TrafficSign("STOP", "trained_detectors/stop_detector.svm",rgb_pixel(255,255,0)));
        signs.push_back(TrafficSign("YEILD", "trained_detectors/yeild_detector.svm",rgb_pixel(255,255,0)));

        std::vector<object_detector<image_scanner_type> > detectors;

        for (int i = 0; i < signs.size(); i++) {
          object_detector<image_scanner_type> detector;
          deserialize(signs[i].svm_path) >> detector;
          detectors.push_back(detector);
        }

        if (parser.option("video")) {
            const std::string video_name = get_option(parser, "video", "road.avi");
            if (process_video(video_name,upsample_amount,signs,detectors))  {
                return EXIT_FAILURE;
            }
        } else  {
            dlib::array<array2d<unsigned char> > images;

            images.resize(parser.number_of_arguments());

            for (unsigned long i = 0; i < images.size(); ++i) {
                load_image(images[i], parser[i]);
            }

            for (unsigned long i = 0; i < upsample_amount; ++i) {
                for (unsigned long j = 0; j < images.size(); ++j) {
                    pyramid_up(images[j]);
                }
            }

            image_window win;
            std::vector<rect_detection> rects;
            for (unsigned long i = 0; i < images.size(); ++i) {
                evaluate_detectors(detectors, images[i], rects);

                // Put the image and detections into the window.
                win.clear_overlay();
                win.set_image(images[i]);

                for (unsigned long j = 0; j < rects.size(); ++j) {
                    win.add_overlay(rects[j].rect, signs[rects[j].weight_index].color,signs[rects[j].weight_index].name);
                }

                if (parser.option("wait")) {
                    cout << "Press any key to continue...";
                    cin.get();
                } else    {
                    sleep((unsigned int)2);
                }
            }
        }
    } catch (exception& e) {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}
