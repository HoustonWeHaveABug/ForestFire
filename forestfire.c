#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define NAME_LEN 9
#define CLASS_LEN 1
#define BACKGROUND_LEN 6
#define DIRECTION_LEN 2
#define STATE_FIRE 4
#define STATE_LAST 6
#define FIRE_CYCLES STATE_LAST-STATE_FIRE
#define FACTORS_OUT 8
#define ERAND_MAX 100
#define DATA_OUTPUT_NONE 0
#define DATA_OUTPUT_TEXT 1
#define DATA_OUTPUT_HTML 2
#define DATA_PX_MIN 300
#define PERCENT_DIGITS 3
#define PERCENT_PRECISION 2

struct state_s {
	int number;
	char name[NAME_LEN+1];
	int symbol;
	char class[CLASS_LEN+1];
	char background[BACKGROUND_LEN+1];
	int propagations[FIRE_CYCLES];
	unsigned long count;
	double percent;
};
typedef struct state_s state_t;

struct fifo_s {
	int **cell_first;
	int **cell_last;
};
typedef struct fifo_s fifo_t;

struct factor_s {
	char direction[DIRECTION_LEN+1];
	int numerator;
	int denominator;
};
typedef struct factor_s factor_t;

void state_usage(const char *);
void reset_fifo(fifo_t *, int **);
int set_propagation(int *);
void test_neighbours(int *, int);
void test_cell(int *, int, factor_t *);
int erand(int);
void add_fifo(fifo_t *, int *);
void html_factors_row(factor_t *, factor_t *, factor_t *);
void html_factor(factor_t *);
void print_output(const char *, ...);
unsigned long test_class(const char *, const char *, unsigned long);
void print_td(const char *, unsigned long, const char *, ...);

int *cells_init, *cells, *cells_out, **cells_fifo, **cells_fifo_out, data_output;
unsigned long rows_max, columns_max, cells_max;
state_t states[STATE_LAST+1] = { { 0, "Water", 'W', "w", "2200AA", { 0, 0 }, 0, 0.0 }, { 1, "Glades", 'G', "g", "EECC88", { 0, 0 }, 0, 0.0 }, { 2, "Trees", 'T', "t", "008844", { 0, 0 }, 0, 0.0 }, { 3, "Buildings", 'B', "b", "000000", { 0, 0 }, 0, 0.0 }, { 4, "Fire", 'F', "f", "AA2200", { 0, 0 }, 0, 0.0 }, { 5, "Embers", 'E', "e", "EE6600", { 0, 0 }, 0, 0.0 }, { 6, "Ashes", 'A', "a", "666666", { 0, 0 }, 0, 0.0 } }, *state_fire = states+STATE_FIRE, *state_last = states+STATE_LAST;
fifo_t *fifo_next;
factor_t factors[FACTORS_OUT] = { { "N", 1, 1 }, { "NE", 1, 1 }, { "E", 1, 1 }, { "ES", 1, 1 }, { "S", 1, 1 }, { "SW", 1, 1 }, { "W", 1, 1 }, { "WN", 1, 1 } };

int main(void) {
int *cell_init, *cell, state_fill, fire_cycle, **cell_fifo;
unsigned long rows_init, columns_init, row, column, cycles, cycle, px;
state_t *state;
fifo_t fifos[2], *fifo, *fifo_tmp;
factor_t *factors_out = factors+FACTORS_OUT, *factor;
	if (scanf("%lu", &rows_init) != 1 || rows_init == 0) {
		fprintf(stderr, "Initial number of rows must be greater than 0.\n");
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &columns_init) != 1 || columns_init == 0) {
		fprintf(stderr, "Initial number of columns must be greater than 0.\n");
		return EXIT_FAILURE;
	}
	fgetc(stdin);
	cells_init = malloc(sizeof(int)*rows_init*columns_init);
	if (!cells_init) {
		fprintf(stderr, "Cannot allocate memory for initial cells.\n");
		return EXIT_FAILURE;
	}
	cell_init = cells_init;
	for (row = 0; row < rows_init; row++) {
		for (column = 0; column < columns_init; column++) {
			*cell_init = fgetc(stdin);
			if (*cell_init < '0' || *cell_init > '0'+STATE_LAST) {
				state_usage("Initial cell");
				free(cells_init);
				return EXIT_FAILURE;
			}
			*cell_init++ -= '0';
		}
		fgetc(stdin);
	}
	if (scanf("%d", &state_fill) != 1 || state_fill < 0 || state_fill > STATE_LAST) {
		state_usage("Fill state");
		free(cells_init);
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &cycles) != 1) {
		fprintf(stderr, "Number of cycles is invalid.\n");
		free(cells_init);
		return EXIT_FAILURE;
	}
	rows_max = rows_init+cycles*2;
	columns_max = columns_init+cycles*2;
	cells_max = rows_max*columns_max;
	cells = malloc(sizeof(int)*cells_max);
	if (!cells) {
		fprintf(stderr, "Cannot allocate memory for cells.\n");
		free(cells_init);
		return EXIT_FAILURE;
	}
	cells_out = cells+cells_max;
	cells_fifo = malloc(sizeof(int *)*cells_max);
	if (!cells_fifo) {
		fprintf(stderr, "Cannot allocate memory for FIFO cells.\n");
		free(cells);
		free(cells_init);
		return EXIT_FAILURE;
	}
	cells_fifo_out = cells_fifo+cells_max;
	reset_fifo(fifos, cells_fifo);
	cell_init = cells_init;
	cell = cells;
	for (row = 0; row < cycles; row++) {
		for (column = 0; column < columns_max; column++) {
			*cell++ = state_fill;
		}
	}
	for (row = 0; row < rows_init; row++) {
		for (column = 0; column < cycles; column++) {
			*cell++ = state_fill;
		}
		for (column = 0; column < columns_init; column++) {
			*cell = *cell_init;
			if (*cell >= STATE_FIRE && *cell < STATE_LAST) {
				add_fifo(fifos, cell);
			}
			cell_init++;
			cell++;
		}
		for (column = 0; column < cycles; column++) {
			*cell++ = state_fill;
		}
	}
	for (row = 0; row < cycles; row++) {
		for (column = 0; column < columns_max; column++) {
			*cell++ = state_fill;
		}
	}
	reset_fifo(fifos+1, fifos->cell_last);
	for (state = states; state < state_fire; state++) {
		for (fire_cycle = 0; fire_cycle < FIRE_CYCLES; fire_cycle++) {
			if (!set_propagation(&state->propagations[fire_cycle])) {
				free(cells_fifo);
				free(cells);
				free(cells_init);
				return EXIT_FAILURE;
			}
		}
	}
	for (factor = factors; factor < factors_out; factor++) {
		if (scanf("%d/%d", &factor->numerator, &factor->denominator) != 2 || factor->numerator < 0 || factor->denominator <= 0) {
			fprintf(stderr, "Wind factor must be a fraction n/d with n >= 0 and d > 0.\n");
			free(cells_fifo);
			free(cells);
			free(cells_init);
			return EXIT_FAILURE;
		}
	}
	if (scanf("%d", &data_output) != 1 || data_output < DATA_OUTPUT_NONE || data_output > DATA_OUTPUT_HTML) {
		fprintf(stderr, "Data output mode must equal %d (no output), %d (text) or %d (html).\n", DATA_OUTPUT_NONE, DATA_OUTPUT_TEXT, DATA_OUTPUT_HTML);
		free(cells_fifo);
		free(cells);
		free(cells_init);
		return EXIT_FAILURE;
	}
	if (data_output == DATA_OUTPUT_HTML) {
		px = 1;
		while (columns_max*px < DATA_PX_MIN) px++;
		puts("<!DOCTYPE HTML>");
		puts("<HTML DIR=\"ltr\" LANG=\"en\">");
		puts("<HEAD>");
		puts("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=utf-8\">");
		puts("<TITLE>Forest Fire Simulation</TITLE>");
		puts("<STYLE TYPE=\"text/css\">");
		puts("BODY { font-family: Verdana, Geneva, Arial, Helvetica, sans-serif; }");
		puts("DIV { overflow: auto; padding: 2px 6px 2px 6px; text-align: center; }");
		puts("DIV.header { background-color: #666666; color: #EEEEEE; }");
		puts("H1 { font-size: 18px; }");
		puts("A { color: #CCCCCC; text-decoration: none; }");
		puts("A:hover { color: #EEEEEE; }");
		puts("DIV.data { background-color: #CCCCCC; }");
		puts("TABLE { background-color: #EEEEEE; border-collapse: collapse; margin: 6px auto 6px auto; }");
		puts("CAPTION { background-color: #666666; color: #EEEEEE; font-size: 16px; font-weight: bold; padding: 2px 6px 2px 6px; }");
		puts("TD.title { font-size: 14px; font-weight: bold; padding: 2px 6px 2px 6px; text-align: center; }");
		puts("TD.left { font-size: 14px; padding: 2px 6px 2px 6px; text-align: left; }");
		puts("TD.right { font-size: 14px; padding: 2px 6px 2px 6px; text-align: right; }");
		puts("TD.center { font-size: 14px; padding: 2px 6px 2px 6px; text-align: center; }");
		for (state = states; state <= state_last; state++) {
			printf("TD.%s { background-color: #%s; height: %lupx; width: %lupx; }\n", state->class, state->background, px, px);
		}
		puts("</STYLE>");
		puts("</HEAD>");
		puts("<BODY>");
		puts("<DIV CLASS=\"header\">");
		puts("<H1>");
		printf("<A HREF=\"https://zestedesavoir.com/forums/sujet/4690/novembre-2015-simulation-dun-feu-de-foret/\" TARGET=\"_blank\">");
	}
	printf("FOREST FIRE SIMULATION");
	printf(data_output == DATA_OUTPUT_HTML ? "</A><BR><BR>":"\n\n");
	printf("Number of cycles planned %lu\n", cycles);
	if (data_output == DATA_OUTPUT_HTML) {
		puts("</H1>");
		puts("</DIV>");
		puts("<DIV CLASS=\"data\">");
		puts("<TABLE>");
		puts("<CAPTION>Propagation ratios</CAPTION>");
		puts("<TR>");
		print_td("title", 1UL, "");
		for (state = state_fire; state < state_last; state++) {
			print_td("title", 1UL, "%s", state->name);
		}
		puts("</TR>");
		for (state = states; state < state_fire; state++) {
			puts("<TR>");
			print_td("left", 1UL, "%s", state->name);
			for (fire_cycle = 0; fire_cycle < FIRE_CYCLES; fire_cycle++) {
				print_td("right", 1UL, "%d%%", state->propagations[fire_cycle]);
			}
			puts("</TR>");
		}
		puts("</TABLE>");
		puts("<TABLE>");
		puts("<CAPTION>Wind factors</CAPTION>");
		html_factors_row(factors+7, factors, factors+1);
		puts("<TR>");
		html_factor(factors+6);
		print_td("center", 1UL, "<IMG ALT=\"Compass\" SRC=\"forestfire_compass.png\" />");
		html_factor(factors+2);
		puts("</TR>");
		html_factors_row(factors+5, factors+4, factors+3);
		puts("</TABLE>");
	}
	else {
		puts("\nPropagation ratios\n");
		printf("%-*s", NAME_LEN, "");
		for (state = state_fire; state < state_last; state++) {
			printf(" %*s", NAME_LEN, state->name);
		}
		puts("\n");
		for (state = states; state < state_fire; state++) {
			printf("%-*s", NAME_LEN, state->name);
			for (fire_cycle = 0; fire_cycle < FIRE_CYCLES; fire_cycle++) {
				printf("%*d%%", NAME_LEN, state->propagations[fire_cycle]);
			}
			puts("");
		}
		puts("\nWind factors\n");
		for (factor = factors; factor < factors_out; factor++) {
			printf("%-*s %d", DIRECTION_LEN, factor->direction, factor->numerator);
			if (factor->denominator > 1) {
				printf("/%d", factor->denominator);
			}
			puts("");
		}
	}
	print_output("Initial map");
	srand((unsigned)time(NULL));
	fifo = fifos;
	fifo_next = fifos+1;
	for (cycle = 0; cycle < cycles && fifo->cell_first != fifo->cell_last; cycle++) {
		for (cell_fifo = fifo->cell_first; cell_fifo != fifo->cell_last && cell_fifo < cells_fifo_out; cell_fifo++) {
			test_neighbours(*cell_fifo, **cell_fifo-STATE_FIRE);
		}
		if (cell_fifo != fifo->cell_last) {
			for (cell_fifo = cells_fifo; cell_fifo != fifo->cell_last; cell_fifo++) {
				test_neighbours(*cell_fifo, **cell_fifo-STATE_FIRE);
			}
		}
		fifo_tmp = fifo;
		fifo = fifo_next;
		fifo_next = fifo_tmp;
		reset_fifo(fifo_next, fifo->cell_last);
	}
	if (cycle > 0) {
		for (state = states; state <= state_last; state++) {
			state->count = 0;
		}
		cycle > 1 ? print_output("Map after %lu cycles", cycle):print_output("Map after 1 cycle");
	}
	if (data_output == DATA_OUTPUT_HTML) {
		puts("</DIV>");
		puts("</BODY>");
		puts("</HTML>");
	}
	free(cells_fifo);
	free(cells);
	free(cells_init);
	return EXIT_SUCCESS;
}

void state_usage(const char *name) {
state_t *state = states;
	fprintf(stderr, "%s must equal %d (%s)", name, state->number, state->name);
	for (state++; state < state_last; state++) {
		fprintf(stderr, ", %d (%s)", state->number, state->name);
	}
	fprintf(stderr, " or %d (%s).\n", state->number, state->name);
}

void reset_fifo(fifo_t *fifo, int **cell) {
	fifo->cell_first = cell;
	fifo->cell_last = cell;
}

int set_propagation(int *propagation) {
	if (scanf("%d", propagation) != 1 || *propagation < 0 || *propagation > ERAND_MAX) {
		fprintf(stderr, "Propagation ratio must lie between 0 and %d.\n", ERAND_MAX);
		return 0;
	}
	else {
		return 1;
	}
}

void test_neighbours(int *cell, int fire_cycle) {
	test_cell(cell-columns_max, fire_cycle, factors);
	test_cell(cell-columns_max+1, fire_cycle, factors+1);
	test_cell(cell+1, fire_cycle, factors+2);
	test_cell(cell+1+columns_max, fire_cycle, factors+3);
	test_cell(cell+columns_max, fire_cycle, factors+4);
	test_cell(cell+columns_max-1, fire_cycle, factors+5);
	test_cell(cell-1, fire_cycle, factors+6);
	test_cell(cell-1-columns_max, fire_cycle, factors+7);
	(*cell)++;
	if (*cell < STATE_LAST) {
		add_fifo(fifo_next, cell);
	}
}

void test_cell(int *cell, int fire_cycle, factor_t *factor) {
	if (cell >= cells && cell < cells_out && *cell < STATE_FIRE && erand(ERAND_MAX) < states[*cell].propagations[fire_cycle]*factor->numerator/factor->denominator) {
		*cell = STATE_FIRE;
		add_fifo(fifo_next, cell);
	}
}

int erand(int values) {
	return (int)(rand()/(RAND_MAX+1.0)*values);
}

void add_fifo(fifo_t *fifo, int *cell) {
	*(fifo->cell_last) = cell;
	fifo->cell_last++;
	if (fifo->cell_last == cells_fifo_out) {
		fifo->cell_last = cells_fifo;
	}
}

void html_factors_row(factor_t *factor_a, factor_t *factor_b, factor_t *factor_c) {
	puts("<TR>");
	html_factor(factor_a);
	html_factor(factor_b);
	html_factor(factor_c);
	puts("</TR>");
}

void html_factor(factor_t *factor) {
	factor->denominator > 1 ? print_td("center", 1UL, "%s %d/%d", factor->direction, factor->numerator, factor->denominator):print_td("center", 1UL, "%s %d", factor->direction, factor->numerator);
}

void print_output(const char *title, ...) {
char class[CLASS_LEN+1], class_old[CLASS_LEN+1];
int *cell;
unsigned long percent, column, row, colspan;
va_list arguments;
state_t *state;
	for (cell = cells; cell < cells_out; cell++) {
		states[*cell].count++;
	}
	for (state = states; state <= state_last; state++) {
		state->percent = state->count*100.0/cells_max;
	}
	va_start(arguments, title);
	if (data_output == DATA_OUTPUT_HTML) {
		puts("<TABLE>");
		printf("<CAPTION>");
		vprintf(title, arguments);
		puts(" - Statistics</CAPTION>");
		for (state = states; state <= state_last; state++) {
			puts("<TR>");
			print_td("left", 1UL, "%s", state->name);
			print_td("right", 1UL, "%lu", state->count);
			print_td("right", 1UL, "%.*f%%", PERCENT_PRECISION, state->percent);
			if (state->count > 0) {
				percent = (unsigned long)ceil(state->percent);
				for (column = 0; column < percent; column++) {
					print_td(state->class, 1UL, "");
				}
			}
			puts("</TR>");
		}
		puts("</TABLE>");
		puts("<TABLE>");
		printf("<CAPTION>");
		vprintf(title, arguments);
		puts(" - Data</CAPTION>");
		cell = cells;
		puts("<TR>");
		for (column = 0; column < columns_max; column++) {
			print_td(states[*cell].class, 1UL, "");
			cell++;
		}
		puts("</TR>");
		for (row = 1; row < rows_max; row++) {
			puts("<TR>");
			strcpy(class_old, "?");
			strcpy(class, states[*cell].class);
			colspan = 1;
			cell++;
			for (column = 1; column < columns_max; column++) {
				strcpy(class_old, class);
				strcpy(class, states[*cell].class);
				colspan = test_class(class, class_old, colspan);
				cell++;
			}
			print_td(class, colspan, "");
			puts("</TR>");
		}
		puts("</TABLE>");
	}
	else {
		puts("");
		vprintf(title, arguments);
		puts("\n");
		for (state = states; state <= state_last; state++) {
			printf("%-*s %10lu %*.*f%%", NAME_LEN, state->name, state->count, PERCENT_DIGITS+1+PERCENT_PRECISION, PERCENT_PRECISION, state->percent);
			if (data_output == DATA_OUTPUT_TEXT && state->count > 0) {
				putchar(' ');
				percent = (unsigned long)ceil(state->percent);
				for (column = 0; column < percent; column++) {
					putchar(state->symbol);
				}
			}
			puts("");
		}
		if (data_output == DATA_OUTPUT_TEXT) {
			puts("");
			cell = cells;
			for (row = 0; row < rows_max; row++) {
				for (column = 0; column < columns_max; column++) {
					putchar(states[*cell].symbol);
					cell++;
				}
				puts("");
			}
		}
	}
	va_end(arguments);
}

unsigned long test_class(const char *class, const char *class_old, unsigned long colspan) {
	if (strcmp(class, class_old)) {
		print_td(class_old, colspan, "");
		return 1;
	}
	else {
		return colspan+1;
	}
}

void print_td(const char *class, unsigned long colspan, const char *format, ...) {
va_list arguments;
	printf("<TD CLASS=\"%s\"", class);
	if (colspan > 1) {
		printf(" COLSPAN=\"%lu\"", colspan);
	}
	printf(">");
	va_start(arguments, format);
	vprintf(format, arguments);
	va_end(arguments);
	puts("</TD>");
}
