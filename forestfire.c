#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct state_s state_t;
struct state_s {
	char name[10];
	unsigned long count;
	double percent;
	int symbol;
	char class[2];
	char background[7];
};

typedef struct cell_s cell_t;
struct cell_s {
	int state;
	cell_t *n;
	cell_t *ne;
	cell_t *e;
	cell_t *es;
	cell_t *s;
	cell_t *sw;
	cell_t *w;
	cell_t *wn;
};

typedef struct fire_s fire_t;
struct fire_s {
	cell_t **cells;
	cell_t **cells_last;
};

void state_usage(const char *);
int init_fire(fire_t *);
void test_neighbours(cell_t *);
void test_neighbour(cell_t *);
int erand(int);
void set_fire(fire_t *, cell_t *);
void print_output(const char *, ...);
unsigned long test_class(const char *, const char *, unsigned long);
void print_td(const char *, unsigned long, const char *, ...);
void free_data(void);

int *cells_init = NULL, propagation, data_output;
unsigned long cells_max, rows_max, columns_max, columns_html;
state_t states[6] = { { "Glades", 0, 0.0, 'G', "g", "EECC88" }, { "Trees", 0, 0.0, 'T', "t", "008844" }, { "Water", 0, 0.0, 'W', "w", "2200AA" }, { "Buildings", 0, 0.0, 'B', "b", "000000" }, { "Fire", 0, 0.0, 'F', "f", "AA2200" }, { "Ashes", 0, 0.0, 'A', "a", "666666" } }, *states_last = states+6;
cell_t *cells = NULL, *cells_last;
fire_t fires[2] = { { NULL, NULL }, { NULL, NULL } }, *fire_next;

int main(void) {
int r, *cell_init, state_fill;
unsigned long rows_init, columns_init, row, column, distance_fill, cycles, cycle, px;
state_t *state;
cell_t *cell, **fire_cell;
fire_t *fire_last, *fire_tmp;
	r = scanf("%lu", &rows_init);
	if (r != 1 || rows_init == 0) {
		fprintf(stderr, "Initial number of rows must be greater than 0.\n");
		return EXIT_FAILURE;
	}
	r = scanf("%lu", &columns_init);
	if (r != 1 || columns_init == 0) {
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
			if (*cell_init < '0' || *cell_init > '5') {
				state_usage("Initial cell");
				free_data();
				return EXIT_FAILURE;
			}
			*cell_init -= '0';
			cell_init++;
		}
		fgetc(stdin);
	}
	r = scanf("%d", &state_fill);
	if (r != 1 || state_fill < 0 || state_fill > 5) {
		state_usage("Fill state");
		free_data();
		return EXIT_FAILURE;
	}
	r = scanf("%lu", &distance_fill);
	if (r != 1) {
		fprintf(stderr, "Fill distance is invalid.\n");
		free_data();
		return EXIT_FAILURE;
	}
	rows_max = rows_init+distance_fill*2;
	columns_max = columns_init+distance_fill*2;
	cells_max = rows_max*columns_max;
	cells = malloc(sizeof(cell_t)*cells_max);
	if (!cells) {
		fprintf(stderr, "Cannot allocate memory for cells.\n");
		free_data();
		return EXIT_FAILURE;
	}
	cells_last = cells+cells_max;
	cell = cells;
	for (row = 1; row <= rows_max; row++) {
		for (column = 1; column <= columns_max; column++) {
			cell->state = state_fill;
			cell->n = row > 1 ? cell-columns_max:NULL;
			cell->ne = row > 1 && column < columns_max ? cell-columns_max+1:NULL;
			cell->e = column < columns_max ? cell+1:NULL;
			cell->es = row < rows_max && column < columns_max ? cell+columns_max+1:NULL;
			cell->s = row < rows_max ? cell+columns_max:NULL;
			cell->sw = row < rows_max && column > 1 ? cell+columns_max-1:NULL;
			cell->w = column > 1 ? cell-1:NULL;
			cell->wn = row > 1 && column > 1 ? cell-columns_max-1:NULL;
			cell++;
		}
	}
	r = init_fire(fires);
	if (!r) {
		free_data();
		return EXIT_FAILURE;
	}
	r = init_fire(fires+1);
	if (!r) {
		free_data();
		return EXIT_FAILURE;
	}
	cell_init = cells_init;
	cell = cells+distance_fill*columns_max+distance_fill;
	for (row = 0; row < rows_init; row++) {
		for (column = 0; column < columns_init; column++) {
			cell->state = *cell_init;
			if (*cell_init == 4) {
				set_fire(fires, cell);
			}
			cell_init++;
			cell++;
		}
		cell += distance_fill*2;
	}
	r = scanf("%d", &propagation);
	if (r != 1 || propagation < 0 || propagation > 100) {
		fprintf(stderr, "Propagation factor must lie between 0 and 100.\n");
		free_data();
		return EXIT_FAILURE;
	}
	r = scanf("%lu", &cycles);
	if (r != 1) {
		fprintf(stderr, "Number of cycles is invalid.\n");
		free_data();
		return EXIT_FAILURE;
	}
	r = scanf("%d", &data_output);
	if (r != 1 || data_output < 0 || data_output > 2) {
		fprintf(stderr, "Data output mode must equal 0 (no output), 1 (text) or 2 (html).\n");
		free_data();
		return EXIT_FAILURE;
	}
	if (data_output == 2) {
		columns_html = columns_max+2;
		px = 1;
		while (columns_html*px < 400) px++;
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
		puts("CAPTION { background-color: #666666; color: #EEEEEE; font-size: 14px; font-weight: bold; padding: 2px 6px 2px 6px; }");
		puts("TD.text { padding: 2px 6px 2px 6px; text-align: left; }");
		puts("TD.number { padding: 2px 6px 2px 6px; text-align: right; }");
		for (state = states; state < states_last; state++) {
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
	printf(data_output == 2 ? "</A><BR><BR>":"\n\n");
	printf("Propagation factor %d%% - Number of cycles planned %lu\n", propagation, cycles);
	if (data_output == 2) {
		puts("</H1>");
		puts("</DIV>");
	}
	print_output("Initial map");
	srand((unsigned)time(NULL));
	fire_last = fires;
	fire_next = fires+1;
	for (cycle = 0; cycle < cycles && fire_last->cells < fire_last->cells_last; cycle++) {
		for (fire_cell = fire_last->cells; fire_cell < fire_last->cells_last; fire_cell++) {
			test_neighbours(*fire_cell);
		}
		fire_tmp = fire_last;
		fire_last = fire_next;
		fire_next = fire_tmp;
		fire_next->cells_last = fire_next->cells;
	}
	for (state = states; state < states_last; state++) {
		state->count = 0;
	}
	if (cycle > 0) {
		cycle > 1 ? print_output("Map after %lu cycles", cycle):print_output("Map after 1 cycle");
	}
	if (data_output == 2) {
		puts("</BODY>");
		puts("</HTML>");
	}
	free_data();
	return EXIT_SUCCESS;
}

void state_usage(const char *name) {
	fprintf(stderr, "%s must equal 0 (glades), 1 (trees), 2 (water), 3 (buildings), 4 (fire) or 5 (ashes).\n", name);
}

int init_fire(fire_t *fire) {
	fire->cells = malloc(sizeof(cell_t *)*cells_max);
	if (!fire->cells) {
		fprintf(stderr, "Cannot allocate memory for fire cells.\n");
		return 0;
	}
	fire->cells_last = fire->cells;
	return 1;
}

void test_neighbours(cell_t *cell) {
	cell->state = 5;
	test_neighbour(cell->n);
	test_neighbour(cell->ne);
	test_neighbour(cell->e);
	test_neighbour(cell->es);
	test_neighbour(cell->s);
	test_neighbour(cell->sw);
	test_neighbour(cell->w);
	test_neighbour(cell->wn);
}

void test_neighbour(cell_t *cell) {
	if (cell && (cell->state == 1 || cell->state == 3) && erand(100) < propagation) {
		cell->state = 4;
		set_fire(fire_next, cell);
	}
}

int erand(int values) {
	return (int)(rand()/(RAND_MAX+1.0)*values);
}

void set_fire(fire_t *fire, cell_t *cell) {
	*(fire->cells_last) = cell;
	fire->cells_last++;
}

void print_output(const char *title, ...) {
char class[2], class_last[2];
unsigned long percent, column, row, colspan;
va_list arguments;
state_t *state;
cell_t *cell;
	for (cell = cells; cell < cells_last; cell++) {
		states[cell->state].count++;
	}
	for (state = states; state < states_last; state++) {
		state->percent = state->count*100.0/cells_max;
	}
	va_start(arguments, title);
	if (data_output == 2) {
		puts("<DIV CLASS=\"data\">");
		puts("<TABLE>");
		printf("<CAPTION>");
		vprintf(title, arguments);
		puts(" - Statistics</CAPTION>");
		for (state = states; state < states_last; state++) {
			puts("<TR>");
			print_td("text", 1UL, "%s", state->name);
			print_td("number", 1UL, "%lu", state->count);
			print_td("number", 1UL, "%.2f%%", state->percent);
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
		puts("<TR>");
		for (column = 0; column < columns_html; column++) {
			print_td("g", 1UL, "");
		}
		puts("</TR>");
		cell = cells;
		for (row = 0; row < rows_max; row++) {
			puts("<TR>");
			strcpy(class, "g");
			strcpy(class_last, "?");
			colspan = 1;
			for (column = 0; column < columns_max; column++) {
				strcpy(class_last, class);
				strcpy(class, states[cell->state].class);
				colspan = test_class(class, class_last, colspan);
				cell++;
			}
			print_td(class, colspan, "");
			print_td("g", 1UL, "");
			puts("</TR>");
		}
		puts("<TR>");
		print_td("g", columns_html, "");
		puts("</TR>");
		puts("</TABLE>");
		puts("</DIV>");
	}
	else {
		puts("");
		vprintf(title, arguments);
		puts("\n");
		for (state = states; state < states_last; state++) {
			printf("%-9s %10lu %6.2f%%\n", state->name, state->count, state->percent);
		}
		if (data_output == 1) {
			puts("");
			cell = cells;
			for (row = 0; row < rows_max; row++) {
				for (column = 0; column < columns_max; column++) {
					printf("%c", states[cell->state].symbol);
					cell++;
				}
				puts("");
			}
		}
	}
	va_end(arguments);
}

unsigned long test_class(const char *class, const char *class_last, unsigned long colspan) {
	if (strcmp(class, class_last)) {
		print_td(class_last, colspan, "");
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

void free_data(void) {
	if (fires[1].cells) {
		free(fires[1].cells);
	}
	if (fires[0].cells) {
		free(fires[0].cells);
	}
	if (cells) {
		free(cells);
	}
	if (cells_init) {
		free(cells_init);
	}
}
