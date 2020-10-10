#include "roadmap_reader.h"
#include "inventory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ExistingRoadmapMove* existingRoadmapMoves;
int roadmapMoves = -1;
int currentMove = -1;

int readRoadmap(FILE* fp, int moves) {
	int totalMoves = -2; // First two lines of file do not contain any moves
	int ch;
	while (!feof(fp)) {
		ch = fgetc(fp);
		if (ch == '\n')
			totalMoves++;
	}

	// Trying to request more moves than the file actually has
	if (totalMoves < moves)
		return 0;

	existingRoadmapMoves = malloc(sizeof(struct ExistingRoadmapMove) * moves);
	roadmapMoves = moves;
	currentMove = 0;

	// Reset file back to beginning
	rewind(fp);

	// Skip first two lines
	char buffer[1024];
	fgets(buffer, sizeof(buffer), fp);
	fgets(buffer, sizeof(buffer), fp);

	int moveIndex = 0;
	while (fgets(buffer, sizeof(buffer), fp)) {
		struct RoadmapLineSearchResult* itemSearchResult = findBetween(buffer, "[", "]", 3);
		struct RoadmapLineSearchResult* handleOutputSearchResult = findBetween(buffer, "(", ")", 1);
		struct RoadmapLineSearchResult* extraSearchResult = findBetween(buffer, "<", ">", 1);


		// There are 3 possible cases that we need to handle, Toss, Autoplace, and TossOther
		// Each has a different combination of the above search results
		struct ExistingRoadmapMove* move = &existingRoadmapMoves[moveIndex];

		// Toss
		// - itemSearchResult will contain 1 or 2 items
		// - handleOutputSearchResult will contain (and toss)
		// - extraSearchResult will contain an item
		if (handleOutputSearchResult->count == 1 && strcmp(handleOutputSearchResult->results[0], "and toss") == 0) {
			move->moveType = Cook;
			move->numValues = itemSearchResult->count;
			move->item1 = getItemFromName(itemSearchResult->results[0]);
			move->item2 = move->numValues > 1 ? getItemFromName(itemSearchResult->results[1]) : -1;
			move->handleOutput = Toss;
			move->output = getItemFromName(extraSearchResult->results[0]);
			move->toss = move->output;
			move->ch5 = NULL;
		}

		// Autoplace
		// - itemSearchResult will contain 1 or 2 items
		// - handleOutputSearchResult will contain (and auto-place)
		// - extraSearchResult will contain an item
		else if (handleOutputSearchResult->count == 1 && strcmp(handleOutputSearchResult->results[0], "and auto-place") == 0) {
			move->moveType = Cook;
			move->numValues = itemSearchResult->count;
			move->item1 = getItemFromName(itemSearchResult->results[0]);
			move->item2 = move->numValues > 1 ? getItemFromName(itemSearchResult->results[1]) : -1;
			move->handleOutput = Autoplace;
			move->output = getItemFromName(extraSearchResult->results[0]);
			move->toss = -1;
			move->ch5 = NULL;
		}

		// TossOther
		// - itemSearchResult will contain 2 or 3 items
		// - handleOutputSearchResult will find nothing
		// - extraSearchResult will contain an item
		else if (itemSearchResult->count > 1 && handleOutputSearchResult->count == 0) {
			move->moveType = Cook;
			move->numValues = itemSearchResult->count - 1;
			move->item1 = getItemFromName(itemSearchResult->results[0]);
			move->item2 = move->numValues > 1 ? getItemFromName(itemSearchResult->results[1]) : -1;
			move->handleOutput = TossOther;
			move->output = getItemFromName(extraSearchResult->results[0]);;
			move->toss = getItemFromName(itemSearchResult->results[move->numValues]);
			move->ch5 = NULL;
		}

		// Chapter 5 break, special case
		else if (strpre(buffer, "Ch.5 Break")) {
			move->moveType = Ch5;
			move->numValues = 0;
			move->item1 = -1;
			move->item2 = -1;
			move->handleOutput = -1;
			move->output = -1;
			move->toss = -1;
			
			struct CH5* ch5 = malloc(sizeof(struct CH5));

			size_t index = 0;
			char* token = strtok(buffer, ",");
			while (token != NULL) {
				char tokenCopy[1024];
				strcpy(tokenCopy, token);

				char* sort = strstr(tokenCopy, "sort");
				if (sort != NULL) {
					ch5->lateSort = index == 3;
					if (strstr(tokenCopy, "(Alpha)") != NULL) {
						ch5->ch5Sort = Sort_Alpha_Asc;
					}
					else if (strstr(tokenCopy, "(Reverse-Alpha)") != NULL) {
						ch5->ch5Sort = Sort_Alpha_Des;
					}
					else if (strstr(tokenCopy, "(Type)") != NULL) {
						ch5->ch5Sort = Sort_Type_Asc;
					}
					else if (strstr(tokenCopy, "(Reverse-Type)") != NULL) {
						ch5->ch5Sort = Sort_Type_Des;
					}
					token = strtok(NULL, ",");
					continue;
				}

				int replacingIndex = 0;
				char* replacing = strstr(tokenCopy, "#");
				if (replacing != NULL)
					replacingIndex = (int)strtol(replacing + 1, replacing + 3, 10) - 1;

				switch (index) {
				case 0:
					ch5->indexDriedBouquet = replacingIndex;
					break;
				case 1:
					ch5->indexCoconut = replacingIndex;
					break;
				case 2:
					ch5->indexKeelMango = replacingIndex;
					break;
				case 3:
					ch5->indexCourageShell = replacingIndex;
					break;
				case 4:
					ch5->indexThunderRage = replacingIndex;
					break;
				}

				token = strtok(NULL, ",");
				index++;
			}

			move->ch5 = ch5;
		}

		// A sort happened
		else {
			if (strpre(buffer, "Sort - Alphabetical")) {
				move->moveType = Sort_Alpha_Asc;
			}
			else if (strpre(buffer, "Sort - Reverse Alphabetical")) {
				move->moveType = Sort_Alpha_Des;
			}
			else if (strpre(buffer, "Sort - Type")) {
				move->moveType = Sort_Type_Asc;
			}
			else if (strpre(buffer, "Sort - Reverse Type")) {
				move->moveType = Sort_Type_Des;
			}
			else {
				// ???
				invalidateRoadmap();
				return 0; // Invalid roadmap
			}

			move->numValues = 0;
			move->item1 = -1;
			move->item2 = -1;
			move->handleOutput = -1;
			move->output = -1;
			move->toss = -1;
			move->ch5 = NULL;
		}

		destroySearchResult(itemSearchResult);
		destroySearchResult(handleOutputSearchResult);
		destroySearchResult(extraSearchResult);

		if (++moveIndex >= moves) // Finished reading all we needed
			break;
	}

	fclose(fp);

	return 1;
}

struct RoadmapLineSearchResult* findBetween(char* buffer, char* open, char* close, int maxAmount) {
	char** results = malloc(sizeof(char*) * maxAmount);
	size_t amountFound = 0;
	char* offs = buffer;
	while (offs != NULL) {
		char* begin = strstr(offs, open);
		if (begin == NULL)
			break;

		char* end = strstr(begin + 1, close);
		if (end == NULL)
			break;

		size_t size = end - begin;
		results[amountFound] = calloc(size, sizeof(char));
		strncpy(results[amountFound], begin + 1, size - 1);
		offs = end + 1;

		if (++amountFound >= maxAmount)
			break;
	}

	// Fill extras with nulls
	for (int i = amountFound; i < maxAmount; i++)
		results[i] = NULL;

	struct RoadmapLineSearchResult* searchResult = malloc(sizeof(struct RoadmapLineSearchResult));
	searchResult->maxAmount = maxAmount;
	searchResult->count = amountFound;
	searchResult->results = results;

	return searchResult;
}

void destroySearchResult(struct RoadmapLineSearchResult* searchResult) {
	for (int i = 0; i < searchResult->maxAmount; i++)
		free(searchResult->results[i]);
	free(searchResult->results);
	free(searchResult);
}

int strpre(char* str, char* pre) {
	return strncmp(pre, str, strlen(pre)) == 0;
}

int isRoadmapMoveAvailable() {
	int available = currentMove < roadmapMoves;
	if (!available && currentMove != -1)
		invalidateRoadmap();
	return available;
}

struct ExistingRoadmapMove* getNextRoadmapMove() {
	return &existingRoadmapMoves[currentMove++];
}

void printRoadmapProgress() {
	printf("%d/%d - ", currentMove, roadmapMoves);
}

void invalidateRoadmap() {
	for (int i = 0; i < roadmapMoves; i++)
		if (existingRoadmapMoves[i].moveType == Ch5)
			free(existingRoadmapMoves[i].ch5);
	free(existingRoadmapMoves);
	currentMove = -1;
	roadmapMoves = -1;
}
