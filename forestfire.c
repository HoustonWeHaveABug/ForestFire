#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef struct state_s state_t;
struct state_s {
	int number;
	char name[10];
	unsigned long count;
	double percent;
	int symbol;
	char class[2];
	char background[7];
};

typedef struct fire_s fire_t;
struct fire_s {
	int **cell_first;
	int **cell_last;
};

void state_usage(const char *);
void reset_fire(fire_t *, int **);
void test_neighbours(int *);
void test_cell(int *);
int erand(int);
void add_fire(fire_t *, int *);
void print_output(const char *, ...);
unsigned long test_class(const char *, const char *, unsigned long);
void print_td(const char *, unsigned long, const char *, ...);

int *cells_init, *cells, *cells_last, **cells_fire, **cells_fire_last, propagation, data_output;
unsigned long cells_max, rows_max, columns_max;
state_t states[6] = { { 0, "Water", 0, 0.0, 'W', "w", "2200AA" }, { 1, "Glades", 0, 0.0, 'G', "g", "EECC88" }, { 2, "Trees", 0, 0.0, 'T', "t", "008844" }, { 3, "Buildings", 0, 0.0, 'B', "b", "000000" }, { 4, "Fire", 0, 0.0, 'F', "f", "AA2200" }, { 5, "Ashes", 0, 0.0, 'A', "a", "666666" } }, *states_last = states+5;
fire_t *fire_next;

int main(void) {
int r, *cell_init, *cell, state_fill, **cell_fire;
unsigned long rows_init, columns_init, row, column, cycles, cycle, px;
state_t *state;
fire_t fires[2], *fire, *fire_tmp;
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
				free(cells_init);
				return EXIT_FAILURE;
			}
			*cell_init++ -= '0';
		}
		fgetc(stdin);
	}
	r = scanf("%d", &state_fill);
	if (r != 1 || state_fill < 0 || state_fill > 5) {
		state_usage("Fill state");
		free(cells_init);
		return EXIT_FAILURE;
	}
	r = scanf("%lu", &cycles);
	if (r != 1) {
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
	cells_last = cells+cells_max;
	cells_fire = malloc(sizeof(int *)*cells_max);
	if (!cells_fire) {
		fprintf(stderr, "Cannot allocate memory for fire cells.\n");
		free(cells);
		free(cells_init);
		return EXIT_FAILURE;
	}
	cells_fire_last = cells_fire+cells_max;
	reset_fire(fires, cells_fire);
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
			if (*cell == 4) {
				add_fire(fires, cell);
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
	reset_fire(fires+1, fires->cell_last);
	r = scanf("%d", &propagation);
	if (r != 1 || propagation < 0 || propagation > 100) {
		fprintf(stderr, "Propagation factor must lie between 0 and 100.\n");
		free(cells_fire);
		free(cells);
		free(cells_init);
		return EXIT_FAILURE;
	}
	r = scanf("%d", &data_output);
	if (r != 1 || data_output < 0 || data_output > 2) {
		fprintf(stderr, "Data output mode must equal 0 (no output), 1 (text) or 2 (html).\n");
		free(cells_fire);
		free(cells);
		free(cells_init);
		return EXIT_FAILURE;
	}
	if (data_output == 2) {
		px = 1;
		while (columns_max*px < 400) px++;
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
		for (state = states; state <= states_last; state++) {
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
	fire = fires;
	fire_next = fires+1;
	for (cycle = 0; cycle < cycles && fire->cell_first != fire->cell_last; cycle++) {
		for (cell_fire = fire->cell_first; cell_fire != fire->cell_last && cell_fire < cells_fire_last; cell_fire++) {
			test_neighbours(*cell_fire);
		}
		if (cell_fire != fire->cell_last) {
			for (cell_fire = cells_fire; cell_fire != fire->cell_last; cell_fire++) {
				test_neighbours(*cell_fire);
			}
		}
		fire_tmp = fire;
		fire = fire_next;
		fire_next = fire_tmp;
		reset_fire(fire_next, fire->cell_last);
	}
	if (cycle > 0) {
		for (state = states; state <= states_last; state++) {
			state->count = 0;
		}
		cycle > 1 ? print_output("Map after %lu cycles", cycle):print_output("Map after 1 cycle");
	}
	if (data_output == 2) {
		puts("</BODY>");
		puts("</HTML>");
	}
	free(cells_fire);
	free(cells);
	free(cells_init);
	return EXIT_SUCCESS;
}

void state_usage(const char *name) {
state_t *state = states;
	fprintf(stderr, "%s must equal %d (%s)", name, state->number, state->name);
	for (state++; state < states_last; state++) {
		fprintf(stderr, ", %d (%s)", state->number, state->name);
	}
	fprintf(stderr, " or %d (%s).\n", state->number, state->name);
}

void reset_fire(fire_t *fire, int **cell) {
	fire->cell_first = cell;
	fire->cell_last = cell;
}

void test_neighbours(int *cell) {
	*cell = 5;
	test_cell(cell-columns_max);
	test_cell(cell-columns_max+1);
	test_cell(cell+1);
	test_cell(cell+1+columns_max);
	test_cell(cell+columns_max);
	test_cell(cell+columns_max-1);
	test_cell(cell-1);
	test_cell(cell-1-columns_max);
}

void test_cell(int *cell) {
	if (cell >= cells && cell < cells_last && (*cell == 2 || *cell == 3) && erand(100) < propagation) {
		*cell = 4;
		add_fire(fire_next, cell);
	}
}

int erand(int values) {
	return (int)(rand()/(RAND_MAX+1.0)*values);
}

void add_fire(fire_t *fire, int *cell) {
	*(fire->cell_last) = cell;
	fire->cell_last++;
	if (fire->cell_last == cells_fire_last) {
		fire->cell_last = cells_fire;
	}
}

void print_output(const char *title, ...) {
char class[2], class_last[2];
int *cell;
unsigned long percent, column, row, colspan;
va_list arguments;
state_t *state;
	for (cell = cells; cell < cells_last; cell++) {
		states[*cell].count++;
	}
	for (state = states; state <= states_last; state++) {
		state->percent = state->count*100.0/cells_max;
	}
	va_start(arguments, title);
	if (data_output == 2) {
		puts("<DIV CLASS=\"data\">");
		puts("<TABLE>");
		printf("<CAPTION>");
		vprintf(title, arguments);
		puts(" - Statistics</CAPTION>");
		for (state = states; state <= states_last; state++) {
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
		cell = cells;
		puts("<TR>");
		for (column = 0; column < columns_max; column++) {
			print_td(states[*cell].class, 1UL, "");
			cell++;
		}
		puts("</TR>");
		for (row = 1; row < rows_max; row++) {
			puts("<TR>");
			strcpy(class_last, "?");
			strcpy(class, states[*cell].class);
			colspan = 1;
			cell++;
			for (column = 1; column < columns_max; column++) {
				strcpy(class_last, class);
				strcpy(class, states[*cell].class);
				colspan = test_class(class, class_last, colspan);
				cell++;
			}
			print_td(class, colspan, "");
			puts("</TR>");
		}
		puts("</TABLE>");
		puts("</DIV>");
	}
	else {
		puts("");
		vprintf(title, arguments);
		puts("\n");
		for (state = states; state <= states_last; state++) {
			printf("%-9s %10lu %6.2f%%\n", state->name, state->count, state->percent);
		}
		if (data_output == 1) {
			puts("");
			cell = cells;
			for (row = 0; row < rows_max; row++) {
				for (column = 0; column < columns_max; column++) {
					printf("%c", states[*cell].symbol);
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
