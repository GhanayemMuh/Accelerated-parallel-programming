#ifndef NBODY_IMPL_H
#define NBODY_IMPL_H

#include "nbody.h"
#include <tbb/tbb.h>
#include <immintrin.h>

struct alignas(32) Particle {
    float x, y, z;    
    float vx, vy, vz; 
    char padding[8];  
};

struct ParticleArrays {
    float x[nParticles], y[nParticles], z[nParticles];
    float vx[nParticles], vy[nParticles], vz[nParticles];
};

ParticleArrays particles2;


float _mm256_reduce_add_ps(__m256 v) {
    __m128 hi = _mm256_extractf128_ps(v, 1);
    __m128 lo = _mm256_castps256_ps128(v);
    lo = _mm_add_ps(lo, hi);
    hi = _mm_movehl_ps(hi, lo);
    lo = _mm_add_ps(lo, hi);
    hi = _mm_shuffle_ps(lo, lo, 1);
    lo = _mm_add_ss(lo, hi);
    return _mm_cvtss_f32(lo);
}

void init_particles_parallel() {
    tbb::parallel_for(0, nParticles, 8, [](int i) {
        __m256 x_init = _mm256_set_ps((float)((i+7) % 15), (float)((i+6) % 15), (float)((i+5) % 15), (float)((i+4) % 15),
                                      (float)((i+3) % 15), (float)((i+2) % 15), (float)((i+1) % 15), (float)(i % 15));
        __m256 y_init = _mm256_set_ps((float)(((i+7) * (i+7)) % 15), (float)(((i+6) * (i+6)) % 15), (float)(((i+5) * (i+5)) % 15), (float)(((i+4) * (i+4)) % 15),
                                      (float)(((i+3) * (i+3)) % 15), (float)(((i+2) * (i+2)) % 15), (float)(((i+1) * (i+1)) % 15), (float)((i * i) % 15));
        __m256 z_init = _mm256_set_ps((float)(((i+7) * (i+7) * 3) % 15), (float)(((i+6) * (i+6) * 3) % 15), (float)(((i+5) * (i+5) * 3) % 15), (float)(((i+4) * (i+4) * 3) % 15),
                                      (float)(((i+3) * (i+3) * 3) % 15), (float)(((i+2) * (i+2) * 3) % 15), (float)(((i+1) * (i+1) * 3) % 15), (float)((i * i * 3) % 15));

        _mm256_storeu_ps(&particles2.x[i], x_init);
        _mm256_storeu_ps(&particles2.y[i], y_init);
        _mm256_storeu_ps(&particles2.z[i], z_init);

        __m256 v_init = _mm256_setzero_ps();
        _mm256_storeu_ps(&particles2.vx[i], v_init);
        _mm256_storeu_ps(&particles2.vy[i], v_init);
        _mm256_storeu_ps(&particles2.vz[i], v_init);
    });
}

void move_particles_parallel() {
    const __m256 softening_vec = _mm256_set1_ps(1e-20f);
    const __m256 dt_vec = _mm256_set1_ps(dt);
    const int tile_size = 1024;  

    tbb::parallel_for(0, nParticles, tile_size, [&](int ti) {
        for (int i = ti; i < std::min(ti + tile_size, nParticles); ++i) {
            __m256 Fx_vec = _mm256_setzero_ps();
            __m256 Fy_vec = _mm256_setzero_ps();
            __m256 Fz_vec = _mm256_setzero_ps();

            __m256 xi_vec = _mm256_set1_ps(particles2.x[i]);
            __m256 yi_vec = _mm256_set1_ps(particles2.y[i]);
            __m256 zi_vec = _mm256_set1_ps(particles2.z[i]);

            for (int tj = 0; tj < nParticles; tj += tile_size) {
                for (int j = tj; j < std::min(tj + tile_size, nParticles); j += 8) {
                    _mm_prefetch((const char*)&particles2.x[j + 8], _MM_HINT_T0);
                    _mm_prefetch((const char*)&particles2.y[j + 8], _MM_HINT_T0);
                    _mm_prefetch((const char*)&particles2.z[j + 8], _MM_HINT_T0);

                    __m256 xj_vec = _mm256_load_ps(&particles2.x[j]);
                    __m256 yj_vec = _mm256_load_ps(&particles2.y[j]);
                    __m256 zj_vec = _mm256_load_ps(&particles2.z[j]);

                    __m256 dx_vec = _mm256_sub_ps(xj_vec, xi_vec);
                    __m256 dy_vec = _mm256_sub_ps(yj_vec, yi_vec);
                    __m256 dz_vec = _mm256_sub_ps(zj_vec, zi_vec);

                    __m256 dist_sqr_vec = _mm256_add_ps(_mm256_mul_ps(dx_vec, dx_vec), softening_vec);
                    dist_sqr_vec = _mm256_add_ps(_mm256_mul_ps(dy_vec, dy_vec), dist_sqr_vec);
                    dist_sqr_vec = _mm256_add_ps(_mm256_mul_ps(dz_vec, dz_vec), dist_sqr_vec);

                    __m256 dist_vec = _mm256_sqrt_ps(dist_sqr_vec);
                    __m256 inv_dist_vec = _mm256_div_ps(_mm256_set1_ps(1.0f), dist_vec);
                    __m256 inv_dist3_vec = _mm256_mul_ps(_mm256_mul_ps(inv_dist_vec, inv_dist_vec), inv_dist_vec);

                    Fx_vec = _mm256_add_ps(Fx_vec, _mm256_mul_ps(dx_vec, inv_dist3_vec));
                    Fy_vec = _mm256_add_ps(Fy_vec, _mm256_mul_ps(dy_vec, inv_dist3_vec));
                    Fz_vec = _mm256_add_ps(Fz_vec, _mm256_mul_ps(dz_vec, inv_dist3_vec));
                }
            }

            particles2.vx[i] += dt * _mm256_reduce_add_ps(Fx_vec);
            particles2.vy[i] += dt * _mm256_reduce_add_ps(Fy_vec);
            particles2.vz[i] += dt * _mm256_reduce_add_ps(Fz_vec);
        }
    });

    tbb::parallel_for(0, nParticles, tile_size, [&](int ti) {
        for (int i = ti; i < std::min(ti + tile_size, nParticles); i += 8) {
            __m256 vx_vec = _mm256_load_ps(&particles2.vx[i]);
            __m256 vy_vec = _mm256_load_ps(&particles2.vy[i]);
            __m256 vz_vec = _mm256_load_ps(&particles2.vz[i]);

            __m256 x_vec = _mm256_load_ps(&particles2.x[i]);
            __m256 y_vec = _mm256_load_ps(&particles2.y[i]);
            __m256 z_vec = _mm256_load_ps(&particles2.z[i]);

            x_vec = _mm256_add_ps(x_vec, _mm256_mul_ps(vx_vec, dt_vec));
            y_vec = _mm256_add_ps(y_vec, _mm256_mul_ps(vy_vec, dt_vec));
            z_vec = _mm256_add_ps(z_vec, _mm256_mul_ps(vz_vec, dt_vec));

            _mm256_store_ps(&particles2.x[i], x_vec);
            _mm256_store_ps(&particles2.y[i], y_vec);
            _mm256_store_ps(&particles2.z[i], z_vec);
        }
    });
}

void get_particle_parallel(int i, OneParticle *p) {
    p->x = particles2.x[i];
    p->y = particles2.y[i];
    p->z = particles2.z[i];
    p->vx = particles2.vx[i];
    p->vy = particles2.vy[i];
    p->vz = particles2.vz[i];
}

#endif 
