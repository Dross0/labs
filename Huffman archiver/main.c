#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "heap.h"

enum mode {
	COMPRESSION = 'c',
	DECOMPRESSION = 'd'
};

typedef struct pair_t {
	uchar symbol;
	uchar *code;
}pair_t;

typedef struct tree_node_t{
	uchar symbol;
	struct tree_node_t * right, *left;
}tree_node_t;

void error(char * err_msg, int err_code);
node_t * make_node_arr(int * frequency_table, int number_of_unique_symbols, const int ALPHABET_SIZE);
int change_frequency(int * frequency_table, uchar * buffer, int size, const int ALPHABET_SIZE);
void print_node(node_t * root);
node_t * connect_node(node_t * node1, node_t * node2);
void make_code_table(pair_t * code_table, node_t * root, uchar *code, int pos, int * i);
uchar * get_symbol_code(const pair_t * code_table, const size_t code_table_size, const uchar symbol);
int power(int a, int n);
uchar convert_binary_to_int(const uchar * byte);
int insert_in_str(uchar * dest, uchar * source, const int count, const int pos);
int coding(const pair_t * code_table, const size_t code_table_size, uchar * res_str, const uchar * buffer, const int size, 
			uchar * byte, const int b_index, int * res_str_index);
void make_compression(node_t * code_tree, pair_t * code_table, const size_t code_table_size, FILE * in, const int file_size);
void save_file_info(node_t * code_tree, const size_t code_table_size, const int file_size, FILE * out);
void decompression(FILE * in);
void compression(FILE * in);
void make_byte_str(uchar * byte_str, uchar * buffer, const int size);
int decoding(tree_node_t * code_tree, uchar * buffer, const int size, const int file_size, int real_file_size, FILE * out);
uchar * dec_to_binary(uchar symbol);
tree_node_t * make_tree(uchar * alphabet_str, const int alphabet_str_size);
tree_node_t * create_node(const int symbol, tree_node_t * right, tree_node_t * left);
uchar * get_byte_alphabet_str(node_t * code_tree, const int code_table_size, FILE * out);
void make_alphabet_str(node_t * tree, uchar * str, int * str_index);
uchar take_symbol_from_str(uchar * str, const int str_size, int pos);
tree_node_t * tr(uchar * str, const int str_size, int  pos);


int main(){
	FILE * in = fopen("in.txt", "rb");
	if (in == NULL) {
		error("Cant open input file", 0);
	}
	char mode = 0;
	fscanf(in, "%c\n", &mode);
	if (mode == COMPRESSION){
		compression(in);
	}
	else if (mode == DECOMPRESSION){
		decompression(in);
	}
	else{
		error("Wrong mode", 3);
	}
	return 0;
}

void compression(FILE * in){
	const int BUFFER_SIZE = 1000;
	const int ALPHABET_SIZE =  256;
	fseek(in, 3, SEEK_SET);
	uchar * buffer = (uchar *)calloc(BUFFER_SIZE, sizeof(uchar));
	int * frequency_table = (int *)calloc(ALPHABET_SIZE, sizeof(int));
	int size = 0;
	int number_of_unique_symbols = 0;
	int file_size = 0;
	while ((size = fread(buffer, sizeof(char), BUFFER_SIZE, in))) {
		number_of_unique_symbols += change_frequency(frequency_table, buffer, size, ALPHABET_SIZE);
		file_size += size;
	}
	if (file_size == 0){
		printf("");
		exit(0);
	}
	free(buffer);
	node_t * arr = make_node_arr(frequency_table, number_of_unique_symbols, ALPHABET_SIZE);
	heap_t * heap = build_heap(arr, number_of_unique_symbols, MIN_HEAP);
	while (heap->size != 1) {
		node_t  min_node1 = extract(heap);
		node_t  min_node2 = extract(heap);
		node_t * connected_node = connect_node(&min_node1, &min_node2);
		add_to_heap(heap, *connected_node);
		free(connected_node);
	}
	free(frequency_table);
	pair_t * code_table = (pair_t *)malloc(sizeof(pair_t) * number_of_unique_symbols);
	node_t tree = extract(heap);
	if (number_of_unique_symbols == 1){
		node_t * connected_node = connect_node(&tree, &tree);
		free(connected_node->right);
		connected_node->right = NULL;
		tree = *connected_node;
	}
	uchar c[100] = { 0 };
	int code_index = 0;
	make_code_table(code_table, &tree, c, 0, &code_index);
	make_compression(&tree, code_table, number_of_unique_symbols, in, file_size);
}

void decompression(FILE * in){
	const int ALPHABET_SIZE =  256;
	const int BUFFER_SIZE = 200;
	int file_size = 0;
	int alphabet_str_size = 0;
	if (fread(&file_size, sizeof(int), 1, in) == 0){
		printf("");
		exit(0);
	}
	fread(&alphabet_str_size, sizeof(int), 1, in);
	int s = (alphabet_str_size % 8 == 0) ? alphabet_str_size / 8 : (alphabet_str_size / 8) + 1;
	uchar * alphabet_str = (uchar *)malloc((s + 1) * sizeof(uchar));
	fread(alphabet_str, sizeof(uchar), s, in);
	tree_node_t * code_tree = make_tree(alphabet_str, alphabet_str_size);
	FILE * out = fopen("out.txt", "wb");
	if (out == NULL){
		error("Cant open output file", 3);
	}
	uchar * buffer = (uchar *)calloc(BUFFER_SIZE + 1, sizeof(uchar));
	int size = 0;
	int real_file_size = 0;
	while ((size = fread(buffer, sizeof(uchar), BUFFER_SIZE, in))){
		real_file_size = decoding(code_tree, buffer, size, file_size, real_file_size, out);
	}
}

tree_node_t * make_tree(uchar * alphabet_str, const int alphabet_str_size){
	int size = (alphabet_str_size % 8 == 0) ? alphabet_str_size / 8 : (alphabet_str_size / 8) + 1;
	int byte_str_size = size * 8;
	uchar * byte_str = (uchar *)calloc(byte_str_size + 1, sizeof(uchar));
	make_byte_str(byte_str, alphabet_str, size);
	tree_node_t * root = tr(byte_str, alphabet_str_size, 0);
	return root;
}

tree_node_t * tr(uchar * str, const int str_size, int pos){
	tree_node_t * root = NULL;
	if (str[pos] == '1'){
		str[pos] = '\0';
		pos++;
		uchar symbol = take_symbol_from_str(str, str_size, pos);
		for (int i = 0; i < 8; ++i){
			str[pos + i] = '\0';
		}
		root = create_node(symbol, 0, 0);
	}
	else if (str[pos] == '0'){
		root = create_node(0, 0, 0);
		root->left = tr(str, str_size, pos + 1);
		int s = 1;
		for (int i = 9; i < str_size - pos; ++i){
			if (str[pos + i] != '\0'){
				s = i;
				break;
			}
		}
		root->right = tr(str, str_size, pos + s);
	}
	str[pos] = '\0';
	return root;
}
	

uchar take_symbol_from_str(uchar * str, const int str_size, int pos){
	uchar byte[9] = {0};
	int j = 0;
	for (int i = pos; j < 8; ++i, ++j){
		byte[j] = str[i];
	}
	return convert_binary_to_int(byte);
}


tree_node_t * create_node(const int symbol, tree_node_t * right, tree_node_t * left){
	tree_node_t * tmp = (tree_node_t *)malloc(sizeof(tree_node_t));
	tmp->symbol = symbol;
	tmp->right = right;
	tmp->left = left;
	return tmp;
}

int decoding(tree_node_t * code_tree, uchar * buffer, const int size, const int file_size, int real_file_size, FILE * out){
	int byte_str_size = size * 8;
	uchar * byte_str = (uchar *)calloc(byte_str_size + 1, sizeof(uchar));
	uchar * res_str = (uchar *)malloc(sizeof(uchar) * (file_size + 1));
	int res_str_index = 0;
	make_byte_str(byte_str, buffer, size);
	tree_node_t * cur = (tree_node_t *)malloc(sizeof(tree_node_t));
	cur = code_tree;
	for (int i = 0; i < byte_str_size && real_file_size < file_size; ++i){
		if (byte_str[i] == '0'){
			cur = cur->right;
		}
		else if (byte_str[i] == '1'){
			cur = cur->left;
		}
		if (cur->symbol != 0){
			res_str[res_str_index++] = cur->symbol;
			real_file_size++;
			cur = code_tree;
		}
	}
	fwrite(res_str, sizeof(uchar), res_str_index, out);
	free(res_str);
	free(byte_str);
	return real_file_size;
}

void make_byte_str(uchar * byte_str, uchar * buffer, const int size){
	int pos = 0;
	for (int i = 0; i < size; ++i){
		uchar * byte = dec_to_binary(buffer[i]);
		pos = insert_in_str(byte_str, byte, 8, pos);
		free(byte);
	}
}

uchar * dec_to_binary(uchar symbol){
	const int BYTE_SIZE = 8;
	uchar * byte = (uchar *)calloc(BYTE_SIZE + 1, sizeof(uchar));
	int i = 0;
	while (i < 8){
		byte[i++] = (symbol % 2) + '0';
		symbol /= 2;
	}
	reverse(byte, BYTE_SIZE);
	return byte;
}

void reverse(uchar * str, const int size){
	uchar tmp = 0;
	for (int i = 0; i < size / 2; ++i){
		tmp = str[i];
		str[i] = str[size - i - 1];
		str[size - i - 1] = tmp;
	}
}


void make_compression(node_t * code_tree, pair_t * code_table, const size_t code_table_size, FILE * in, const int file_size) {
	const int BYTE_SIZE = 8;
	const int BUFFER_SIZE = 1000;
	fseek(in, 3, SEEK_SET);
	uchar * res_str = (uchar *)calloc(BUFFER_SIZE + 1, sizeof(uchar));
	uchar * buffer = (uchar *)calloc(BUFFER_SIZE + 1, sizeof(uchar));
	uchar * byte = (uchar *)calloc(BYTE_SIZE + 1, sizeof(uchar));
	FILE * out = fopen("out.txt", "wb");
	if (out == NULL){
		error("Cant open output file", 1);
	}
	save_file_info(code_tree, code_table_size, file_size, out);
	int byte_index = 0;
	int size = 0;
	int res_str_index = 0;
	while ((size = fread(buffer, sizeof(uchar), BUFFER_SIZE, in))) {
		byte_index = coding(code_table, code_table_size, res_str, buffer, size, byte, byte_index, &res_str_index);
		fwrite(res_str, sizeof(uchar), res_str_index, out);
		if (size < BUFFER_SIZE){
			byte_fill(byte, byte_index, BYTE_SIZE);
			uchar sym = convert_binary_to_int(byte);
			fwrite(&sym, sizeof(uchar), 1, out);
		}
		memset(res_str, 0, sizeof(uchar) * res_str_index);		
	}
	fclose(out);
}

void save_file_info(node_t * code_tree, const size_t code_table_size, const int file_size, FILE * out){
	fwrite(&file_size, sizeof(int), 1, out);
	uchar * alphabet_str = get_byte_alphabet_str(code_tree, code_table_size, out);
	int alphabet_str_size = strlen(alphabet_str);
	fwrite(alphabet_str, sizeof(uchar), alphabet_str_size, out);
}

uchar * get_byte_alphabet_str(node_t * code_tree, const int code_table_size, FILE * out){
	uchar * str = (uchar *)calloc((code_table_size * 8) + 200, sizeof(uchar));
	int ind = 0;
	make_alphabet_str(code_tree, str, &ind);
	int size = strlen(str);
	uchar * res_str = (uchar *)calloc(size / 8 + 2, sizeof(uchar));
	int res_str_index = 0; 
	fwrite(&size, sizeof(int), 1, out);
	uchar byte[9] = {0};
	int byte_ind = 0;
	uchar symbol = 0;
	for (int i = 0; i < size; ++i){
		byte[byte_ind++] = str[i];
		if (byte_ind == 8){
			symbol = convert_binary_to_int(byte);
			res_str[res_str_index++] = symbol;
			memset(byte, 0, 9);
			byte_ind = 0;
		}
	}
	if (byte_ind != 0){
		byte_fill(byte, byte_ind, 8);
		symbol = convert_binary_to_int(byte);
		res_str[res_str_index++] = symbol;
	}
	return res_str;
}

void make_alphabet_str(node_t * tree, uchar * str, int * str_index){
	if (tree->symbol == 0){
		str[(*str_index)++] = '0';
		if (tree->left){
			make_alphabet_str(tree->left, str, str_index);
		}
		if (tree->right){
			make_alphabet_str(tree->right, str, str_index);
		}
	}
	else{
		str[(*str_index)++] = '1';
		uchar * byte = dec_to_binary(tree->symbol);
		*str_index = insert_in_str(str, byte, 8, *str_index);
	}
}

void byte_fill(uchar * byte, const int byte_index, const int BYTE_SIZE){
	int b_ind = byte_index;
	for (b_ind; b_ind < BYTE_SIZE; ++b_ind){
		byte[b_ind] = '0';
	}
}


int coding(const pair_t * code_table, const size_t code_table_size, uchar * res_str, const uchar * buffer, const int size, 
			uchar * byte, const int b_index, int * res_str_index) {
	const int BYTE_SIZE = 8;
	int byte_index = b_index;
	uchar * symbol_code = 0;
	int code_size = 0;
	char res_symbol = 0;
	for (int i = 0; i < size; ++i) {
		symbol_code = get_symbol_code(code_table, code_table_size, buffer[i]);
		code_size = strlen(symbol_code);
		if (code_size + byte_index <= BYTE_SIZE) { 																// maybe <
			byte_index = insert_in_str(byte, symbol_code, code_size, byte_index);
		}
		else {
			int count_to_save = code_size - (BYTE_SIZE - byte_index);
			byte_index = insert_in_str(byte, symbol_code, BYTE_SIZE - byte_index, byte_index);
			res_symbol = convert_binary_to_int(byte);
			res_str[(*res_str_index)++] = res_symbol;
			memset(byte, 0, sizeof(char) * BYTE_SIZE);
			for (byte_index = 0; byte_index < count_to_save; ++byte_index){
				byte[byte_index] = symbol_code[code_size - count_to_save + byte_index];
			}
		}
	}
	return byte_index;
}

int insert_in_str(uchar * dest, uchar * source, const int count, const int pos) {
	int i = 0;
	int position = pos;
	while (i < count) {
		dest[position] = source[i++];
		position++;
	}
	return position;
}

uchar convert_binary_to_int(const uchar * byte) {
	uchar symbol = 0;
	int i = 0;
	int size = strlen(byte);
	for (i = 0; i < size; ++i) {
		if (byte[i] == '1') {
			symbol += power(2, size - 1 - i);
		}
	}
	return symbol;
}

int power(int a, int n) {
	if (n == 0) {
		return 1;
	}
	else if (n % 2 == 0) {
		return power(a*a, n / 2);
	}
	return a * power(a, n - 1);
}


uchar * get_symbol_code(const pair_t * code_table, const size_t code_table_size, const uchar symbol) {
	int i = 0;
	for (i = 0; i < code_table_size; ++i) {
		if (code_table[i].symbol == symbol) {
			return code_table[i].code;
		}
	}
}

void make_code_table(pair_t * code_table, node_t * root, uchar *code, int pos, int * code_index) {
	if (root) {
		if (!root->right && !root->left) {
			pair_t pair;
			pair.code = (char *)calloc(pos + 1, sizeof(char));
			memcpy(pair.code, code, pos);
			pair.symbol = root->symbol;
			code_table[*code_index] = pair;
			*code_index += 1;
		}
		if (root->right) {
			code[pos] = '0';
			make_code_table(code_table, root->right, code, pos+1, code_index);
		}
		if (root->left) {
			code[pos] = '1';
			make_code_table(code_table, root->left, code, pos + 1, code_index);
		}
	}
}


node_t * connect_node(node_t * node1, node_t * node2) {
	node_t * res_node = (node_t *)malloc(sizeof(node_t));
	node_t *n1 = (node_t *)malloc(sizeof(node_t));
	node_t *n2 = (node_t *)malloc(sizeof(node_t));
	memcpy(n1, node1, sizeof(node_t));
	memcpy(n2, node2, sizeof(node_t));
	res_node->right = n1;
	res_node->left = n2;
	res_node->frequency = n1->frequency + n2->frequency;
	res_node->symbol = 0;
	return res_node;
}

node_t * make_node_arr(int * frequency_table, int number_of_unique_symbols, const int ALPHABET_SIZE) {
	node_t * arr = (node_t *)malloc(sizeof(node_t) * number_of_unique_symbols);
	int i = 0;
	int j = 0;
	node_t node;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
		if (frequency_table[i]) {
			node.frequency = frequency_table[i];
			node.symbol = i;
			node.left = NULL;
			node.right = NULL;
			arr[j++] = node;
		}
	}
	return arr;
}

int change_frequency(int * frequency_table, uchar * buffer, int size, const int ALPHABET_SIZE) {
	int i = 0;
	int unique_symbols = 0;
	for (i = 0; i < size; ++i) {
		if (buffer[i] < 0 || buffer[i] >= ALPHABET_SIZE) {
			error("Wrong symbol", 0);
		}
		if (frequency_table[buffer[i]] == 0) {
			unique_symbols++;
		}
		frequency_table[buffer[i]]++;
	}
	return unique_symbols;
}

void error(char * err_msg, int err_code) {
	printf("%s %d\n", err_msg, err_code);
	exit(err_code);
}
