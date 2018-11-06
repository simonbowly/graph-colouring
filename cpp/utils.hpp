
#ifndef UTILS_HPP
#define UTILS_HPP

#include "math.h"

#include "gsl/span"
#include "igraph/igraph.h"


namespace graph {


    class IGraphVector {

        bool initialised;
        igraph_vector_t v;
        gsl::span<const igraph_real_t> vec_span;   // possible to just inherit from this?

    public:

        IGraphVector() : initialised(false) {}

        IGraphVector(igraph_vector_t _v) : initialised(true), v(_v) {
            vec_span = gsl::span<const igraph_real_t>{VECTOR(v), igraph_vector_size(&v)};
        }

        ~IGraphVector() {
            if (initialised) {
                igraph_vector_destroy(&v);
            }
        }

        IGraphVector(const IGraphVector&) = delete;
        IGraphVector& operator=(const IGraphVector&) = delete;

        IGraphVector(IGraphVector&& a) {
            initialised = a.initialised;        a.initialised = false;
            v = a.v;                            a.v = igraph_vector_t();
            vec_span = a.vec_span;              a.vec_span = gsl::span<const igraph_real_t>{};
        }

        IGraphVector& operator=(IGraphVector&& a) = delete;

        int size() const { return vec_span.size(); }
        gsl::span<const igraph_real_t>::iterator begin() const { return vec_span.begin(); }
        gsl::span<const igraph_real_t>::iterator end() const { return vec_span.end(); }
        const igraph_real_t& operator[](int i) const { return vec_span[i]; }

        const igraph_vector_t* get_ptr() const { return &v; }

    };


    class IGraphVSIterator {

        bool initialised;
        igraph_vs_t vs;
        igraph_vit_t vit;
        gsl::span<const igraph_real_t> vit_span;

    public:

        IGraphVSIterator() : initialised(false) {}

        IGraphVSIterator(igraph_vs_t _vs, igraph_vit_t _vit) : initialised(true), vs(_vs), vit(_vit) {
            vit_span = gsl::span<const igraph_real_t>{VECTOR(*(vit).vec), vit.end - vit.start};
        }

        ~IGraphVSIterator() {
            if (initialised) {
                igraph_vit_destroy(&vit);
                igraph_vs_destroy(&vs);
            }
        }

        IGraphVSIterator(const IGraphVSIterator&) = delete;
        IGraphVSIterator& operator=(const IGraphVSIterator&) = delete;

        IGraphVSIterator(IGraphVSIterator&& a) {
            initialised = a.initialised;    a.initialised = false;
            vs = a.vs;                      a.vs = igraph_vs_t();
            vit = a.vit;                    a.vit = igraph_vit_t();
            vit_span = a.vit_span;          a.vit_span = gsl::span<const igraph_real_t>{};
        }

        IGraphVSIterator& operator=(IGraphVSIterator&& a) = delete;

        int size() const { return vit_span.size(); }
        gsl::span<const igraph_real_t>::iterator begin() const { return vit_span.begin(); }
        gsl::span<const igraph_real_t>::iterator end() const { return vit_span.end(); }
        const igraph_real_t& operator[](int i) const { return vit_span[i]; }

    };

    template<class T>
    std::pair<double, double> simple_statistics(T d) {

        double n = d.size();

        double mean = 0.0;
        for (const auto & vd : d) {
            mean += vd;
        }
        mean /= n;

        double stdev = 0.0;
        for (const auto & vd : d) {
            stdev += (vd - mean) * (vd - mean);
        }
        stdev = sqrt(stdev / (n - 1.0));

        return std::make_pair(mean, stdev);
    }

}


#endif
