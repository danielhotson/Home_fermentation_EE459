/*
  structs.c - Establishes object structure commands
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "structs.h"                // Declarations of the struct functions

struct Ingredients {
	int water;
	int tea;
	int sugar;
	int scoby;
	int starterL;
};

struct Thresholds {
	int temp;
	int alc;
	int pressure;
	int light;
}

void Ingredients_init(Ingredients* self, int w, int t, int su, int sc, int sL){
	self->water = w;
	self->tea = t;
	self->sugar = su;
	self->scoby = sc;
	self->starterL = sL;
}

Ingredients* Ingredients_create(int w, int t, int su, int sc, int sL){
	Ingredients* result = (Ingredients*) malloc(sizeof(Ingredients));
	Ingredients_init(result, w, t, su, sc, sL);
	return result;
}

void Ingredients_reset(Ingredients* self){
}
void Ingredients_destroy(Ingredients point){
	if (point) {
		Point_reset(point);
		free(point);
	}
}

int water(Ingredients* self){
	return self->water;
}

int tea(Ingredients* self){
	return self->tea;
}

int sugar(Ingredients* self){
	return self->sugar;
}

int scoby(Ingredients* self){
	return self->scoby;
}

int sL(Ingredients* self){
	return self->starterL;
}

void Thresholds_init(Thresholds* self, int t, int a, int p, int l){
	self->temp = t;
	self->alc = a;
	self->pressure = p;
	self->light = l;
}

Thresholds* Thresholds_create(int t, int a, int p, int l){
	Thresholds* result = (Thresholds*) malloc(sizeof(Thresholds));
	Thresholds_init(result, t, a, p, l);
	return result;
}

void Thresholds_reset(Thresholds* self){
}
void Thresholds_destroy(Thresholds point){
	if (point) {
		Point_reset(point);
		free(point);
	}
}

int temp(Thresholds* self){
	return self->temp;
}

int alc(Thresholds* self){
	return self->alc;
}

int pressure(Thresholds* self){
	return self->pressure;
}

int light(Thresholds* self){
	return self->light;
}