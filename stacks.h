/* switch to the specified stack, copying the specified number of arguments.
 * returns on the same stack from which it is called.  To get the return value of
 * the called functions, casting must be done (will not work for functions that return
 * structures via a hidden parameter (unless you get tricky, of course)).
 */
void call_on_stack(void *top_of_stack, void (*function)(...),int numargs, ...);
