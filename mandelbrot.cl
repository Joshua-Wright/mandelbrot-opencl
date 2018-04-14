// same mandelbrot kernel, but with subsampling and gamma correction
#define MAX_ITERS (1<<13)
#define LOG_2 0.693147180559945309417232121458176568075500134360255254120
#define PI2 6.283185307179586476925286766559005768394338798750211641949
#define threshold2 100
#define log_maxVal 31
#define wrap 30
#define dist_est_factor (2<<9)
#define factor 2
#define gamma 1.5

inline double2 cmult(double2 a, double2 b) {
    return (double2)( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

inline double2 csqr(double2 a) {
    return (double2)( a.x*a.x - a.y*a.y, a.x*a.y*2);
}

float4 mandelbrot(double2 coords, double pixel_width) {

    float3 outcolor = (float3)(0,0,0);
    for (int idx1 = 0; idx1 < factor; idx1++) {
        for (int idx2 = 0; idx2 < factor; idx2++) {
            
            double2 z = (double2)(0, 0);
            double2 dz = (double2)(0, 0);
            double2 c = coords + (double2)(idx2,idx1)*pixel_width/factor;

            double val = 0.0;
            unsigned i;
            for (i = 0; i < MAX_ITERS; i++) {
                dz = 2.0f * cmult(z,dz) + 1.0f;
                z = csqr(z) + c;
                if (dot(z,z) > threshold2) {
                    double log_zn = log(z.x*z.x+z.y*z.y) / 2.0f;
                    double nu = log(log_zn/LOG_2) / LOG_2;
                    val = i + 1 - nu;
                    break;
                }
            }
            if (i == MAX_ITERS) {
                continue;
            }

            double zmag2 = z.x*z.x + z.y*z.y;
            double zmag = sqrt(zmag2);
            double dzmag = sqrt(dz.x*dz.x + dz.y*dz.y);
            double dist_est = log(zmag2) * zmag / dzmag;

            if (dist_est < (1 / wrap / dist_est_factor)) {
                continue;
            }

            val = log(val) / log_maxVal * wrap;
            val = sin(val * PI2)*0.5f + 0.5f;
            int vali = val*NUM_COLORS;

            outcolor += pow(colormap[vali].xyz, gamma);
        }
    }
    return (float4)(pow(outcolor / (factor*factor), 1/gamma), 1);
}