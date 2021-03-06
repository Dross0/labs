#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mystack.h"

void error(int flag){	
	flag ? printf("syntax error") :	printf("division by zero");
	exit(0);
}

int get_priority(char sign){
	if (sign == '*' || sign == '/')
		return 4;
	else if (sign == '+' || sign == '-')
		return 3;
	else if (sign == '=')
		return 2;
	else if (sign == '(' || sign == ')')
		return 1;
}

char *get_postfix_form(Node_t *head, char str[], int str_len, int arr[]){
	char *postfix_form = (char *)malloc(sizeof(char) * (str_len + 1));
	memset(postfix_form, 0, sizeof(char) * (str_len + 1));
	char number[MAX_SIZE] = {0};
	int postfix_index = 0;
	int flag = 0;
	int k = 0;
	int sign_count = 0;
	int number_count = 0;
	int open_brackets = 0;
	int close_brackets = 0;
	for (int i = 0; i < str_len; i++){
		if (isdigit(str[i])){
			while (isdigit(str[i])){
				number[k++] = str[i++];
			}
			postfix_form[postfix_index++] = 'n';
			i--;
			int n = atoi(number);
			arr[number_count++] = n;
			memset(number, 0, sizeof(char) * k);
			k = 0;
			flag = 0;
		}
		else if (str[i] == '('){
			push_node(&head, str[i]);			
			flag = 1;
			open_brackets++;
		}
		else if (str[i] == ')'){
			if (flag)
				error(1);			
			while (!is_empty_node(head) && peek_node(head) != '(')
				postfix_form[postfix_index++] = pop_node(&head);
			if (is_empty_node(head))
				error(1);
			pop_node(&head);
			close_brackets++;
		}
		else if (str[i] == '+' || str[i] == '-' || str[i] == '/' || str[i] == '*') {
			while (!is_empty_node(head) && get_priority(peek_node(head)) >= get_priority(str[i]))
				postfix_form[postfix_index++] = pop_node(&head);
			push_node(&head, str[i]);
			sign_count++;
			flag = 0;
		}
		else
			error(1);
	}
	if (number_count != sign_count + 1 || open_brackets != close_brackets)
		error(1);
	while (!is_empty_node(head))
		postfix_form[postfix_index++] = pop_node(&head);
	return postfix_form;
}

int calculate(char str[], int str_len, int arr[], Node_t * head){
	int j = 0;
	for (int i = 0; i < str_len; i++){
		if (str[i] == 'n')
			push_node(&head, arr[j++]);
		else{
			int num1 = pop_node(&head);
			int num2 = pop_node(&head);
			if (str[i] == '+')
				push_node(&head, num1 + num2);
			else if (str[i] == '-')
				push_node(&head, num2 - num1);
			else if (str[i] == '/'){
				if (!num1)
					error(0);
				push_node(&head, num2 / num1);
			}
			else if (str[i] == '*')
				push_node(&head, num1 * num2);
			}	
	}
	return pop_node(&head);
}

int main(){
	char str[MAX_SIZE + 1] = {0};
	gets(str);
	int str_len = strlen(str);
	int arr[MAX_SIZE] = {0};
	Node_t *head = NULL; 
	char *postfix_form = get_postfix_form(head, str, str_len, arr);
	int postfix_form_len = strlen(postfix_form);
	int result = calculate(postfix_form, postfix_form_len, arr, head);
	printf("%d", result);
	free(postfix_form);
	return 0;
}