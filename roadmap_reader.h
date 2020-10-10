#ifndef ROADMAP_READER_H
#define ROADMAP_READER_H

#include "calculator.h"

struct ExistingRoadmapMove {
	enum Action moveType;
	int numValues;
	enum Type_Sort item1;
	enum Type_Sort item2;
	enum HandleOutput handleOutput;
	enum Type_Sort output;
	enum Type_Sort toss;
	struct CH5* ch5;
};

struct RoadmapLineSearchResult {
	int maxAmount;
	int count;
	char** results;
};

int readRoadmap(FILE* fp, int moves);
struct RoadmapLineSearchResult* findBetween(char* buffer, char* open, char* close, int maxAmount);
void destroySearchResult(struct RoadmapLineSearchResult* searchResult);
int strpre(char* str, char* pre);
int isRoadmapMoveAvailable();
struct ExistingRoadmapMove* getNextRoadmapMove();
void printRoadmapProgress();
void invalidateRoadmap();

#endif
