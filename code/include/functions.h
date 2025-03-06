#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cstdint>
#include <vector>

std::vector<uint32_t> findFactors(const uint32_t Value);
bool findOptimalRatio(const uint32_t *PROCESS_COUNT, const int32_t *X_SIZE,
                      const int32_t *Y_SIZE, uint32_t *xDiv, uint32_t *yDiv);
uint32_t digitsInValue(const uint32_t VALUE);
void printIntro();
void printOutro();

#endif // !FUNCTIONS_H
