#include "StringMetrics.h"
#include <iostream>
#include <cmath>

unsigned int
LongestCommonSubsequenceLength(const char *s, unsigned int m, const char *t, unsigned int n)
{
       double d[m + 1][n + 1];
       unsigned int i, j;

       for (i = 0; i <= m; i++) {
               d[i][0] = 0.0;
       }
       for (j = 0; j <= n; j++) {
               d[0][j] = 0.0;
       }
       for (j = 1; j <= n; j++) {
               for (i = 1; i <= m; i++) {
                       if (s[i - 1] == t[j - 1]) {
                               d[i][j] = d[i-1][j-1] + 1;
                       } else {
                               d[i][j] = std::max(d[i][j - 1], d[i-1][j]);
                       }
               }
       }
       return d[m][n];
}

unsigned int
SmithWatermanDistance(const char *s, unsigned int m, const char *t, unsigned int n)
{
       double d[m + 1][2];
       unsigned int i, j;
       double cost;
       double D;

       D = 0;
       for (i = 0; i <= m; i++) {
               d[i][0] = 0.0;
       }
       d[0][0] = 0.0;
       for (j = 1; j <= n; j++) {
               d[0][j % 2] = 0.0;
               for (i = 1; i <= m; i++) {
                       const static double G = 1.0; // gap cost

                       if (s[i - 1] == t[j - 1])
                               cost =  1.0;
                       else {
                               cost =  0.0;
                       }
                       d[i][j % 2] = std::max(
                               0.0,
// start over
                               std::max(d[i - 1][(j - 1) % 2] + cost,
// substitution
                               std::max(d[i - 1][j % 2] - G,
// insertion
                               d[i][(j - 1) % 2] - G)));
// deletion
                       if (d[i][j % 2] > D)
                               D = d[i][j % 2];
               }
       }
       return (int)std::max(m, n) - D;
}

unsigned int
LevenshteinDistance(const char *s, unsigned int m, const char *t, unsigned int n)
{
       unsigned int d[m + 1][2];
       unsigned int i, j;
       unsigned int cost;

       for (i = 0; i <= m; i++) {
               d[i][0] = i;
       }
       d[0][0] = 0;
       for (j = 1; j <= n; j++) {
               d[0][j % 2] = j;
               for (i = 1; i <= m; i++) {
                       if (s[i - 1] == t[j - 1])
                               cost = 0;
                       else
                               cost = 1;
                       d[i][j % 2] = std::min(std::min(
                               d[i - 1][j % 2] + 1,            // insertion
                               d[i][(j - 1) % 2] + 1),         // deletion
                               d[i - 1][(j - 1) % 2] + cost);  // substitution
               }
       }
       return d[m][n % 2];
}
