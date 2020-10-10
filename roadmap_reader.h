#ifndef ROADMAP_READER_H
#define ROADMAP_READER_H

#include "calculator.h"

struct ExistingRoadmapMove {
	enum Action moveType;
	int numValues;
	enum Sort_Type item1;
	enum Sort_Type item2;
	enum HandleOutput handleOutput;
	enum Sort_Type output;
	enum Sort_Type toss;
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
