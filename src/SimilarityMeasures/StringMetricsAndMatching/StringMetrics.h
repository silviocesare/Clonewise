#ifndef StringMetrics_h
#define StringMetrics_h

unsigned int LongestCommonSubsequenceLength(const char *s, unsigned int m, const char *t, unsigned int n);
unsigned int SmithWatermanDistance(const char *s, unsigned int m, const char *t, unsigned int n);
unsigned int LevenshteinDistance(const char *s, unsigned int m, const char *t, unsigned int n);

#endif
