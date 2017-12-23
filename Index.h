//
// Created by dbaranchuk on 23.12.17.
//

#ifndef IVF_HNSW_LIB_INDEX_H
#define IVF_HNSW_LIB_INDEX_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>

#include <faiss/ProductQuantizer.h>
#include <faiss/utils.h>

#include <hnswlib/hnswalg.h>
#include "utils.h"

typedef unsigned int idx_t;
typedef unsigned char uint8_t;

namespace ivfhnsw {
    /** Abstract structure for an index
    *
    * Supports adding vertices and searching them.
    *
    * Currently only asymmetric queries are supported:
    * database-to-database queries are not implemented.
    */
    struct Index
    {
        size_t d;             /** Vector Dimension **/
        size_t nc;            /** Number of Centroids **/
        size_t code_size;     /** PQ Code Size **/

        /** Coarse Quantizer based on HNSW [Y.Malkov]**/
        hnswlib::HierarchicalNSW *quantizer;

        /** Fine Product Quantizers **/
        faiss::ProductQuantizer *norm_pq;
        faiss::ProductQuantizer *pq;

        /** Search Parameters **/
        size_t nprobe = 16;
        size_t max_codes = 10000;

        /** Query Table **/
        std::vector<float> query_table;

    protected:
        std::vector<float> norms;           /** Reconstructed vectors L2 square norms **/
        std::vector<float> centroid_norms;  /** Region centroids L2 square norms **/

    public:
        Index(size_t dim, size_t ncentroids, size_t bytes_per_code, size_t nbits_per_idx);
        virtual ~Index();

        /** Construct HNSW Coarse Quantizer **/
        virtual void buildCoarseQuantizer(const char *path_clusters,
                                          const char *path_info, const char *path_edges,
                                          int M, int efConstruction);


        /** return the indexes of the k vectors closest to the query x.
         *
         * This function is identical as search but only return labels of neighbors.
         * @param x           input vectors to search, size n * d
         * @param labels      output labels of the NNs, size n*k
         */
        virtual void assign(size_t n, const float *data, idx_t *idxs);

        /** query n vectors of dimension d to the index.
         *
         * return at most k vectors. If there are not enough results for a
         * query, the result array is padded with -1s.
         *
         * @param x           input vectors to search, size n * d
         * @param labels      output labels of the NNs, size n*k
         * @param distances   output pairwise distances, size n*k
         */
        virtual void search(float *x, idx_t k, float *distances, long *labels) = 0;

        virtual void train_pq(const size_t n, const float *x) = 0;

        virtual void write(const char *path_index) = 0;
        virtual void read(const char *path_index) = 0;

        virtual void compute_centroid_norms();

    protected:
        virtual float fstdistfunc(uint8_t *code);

        //void reconstruct(size_t n, float *x, const float *decoded_residuals, const idx_t *keys) = 0;
        //void compute_residuals(size_t n, const float *x, float *residuals, const idx_t *keys) = 0;
    };
}
#endif //IVF_HNSW_LIB_INDEX_H