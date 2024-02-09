/**************************************************************************
 * C S 429 EEL interpreter
 *
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 *
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

#include "ci.h"

/* Function interfaces */

/* Returns true if the given token is a binary operator and false otherwise */
extern bool is_binop(token_t);
/* Returns true if the given token is a unary operator and false otherwise */
extern bool is_unop(token_t);
/* It might be helpful to note that TOK_QUESTION is the only ternary operator. */

char *strrev(char *str);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr)
{
    // check running status - you can ignore this
    if (terminate || ignore_input)
        return;

    // Week 1 TODO: Implement a recursive post-order traversal of the AST. Remember to include a base case.
    if (nptr)
    {
        int numChildren = 3;
        for (int i = 0; i < numChildren; ++i)
        {
            if (nptr->children[i])
            {
                infer_type(nptr->children[i]);
            }
        }

        switch (nptr->node_type)
        {
        // For each week, you will also need to include error checking for each type.
        // Week 1 TODO: Implement type inference for all operators on int and bool types.
        // Week 2 TODO: Extend type inference to handle operators on string types.
        // Week 3 TODO: Implement tpye evaluation for variables.
        case NT_INTERNAL:
            switch (nptr->tok)
            {
            // For reference, the identity (do nothing) operator is implemented for you.
            case TOK_IDENTITY:
                nptr->type = nptr->children[0]->type;
                break;

            case TOK_AND: // &
            case TOK_OR:  // |
                if ((nptr->children[1]->type != BOOL_TYPE) || (nptr->children[0]->type != BOOL_TYPE))
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->type = BOOL_TYPE;
                break;

            case TOK_LT: // <
            case TOK_GT: // >
                if (((nptr->children[1]->type == INT_TYPE) && (nptr->children[0]->type == INT_TYPE)))
                {
                    nptr->type = BOOL_TYPE;
                    break;
                }
                else if (((nptr->children[1]->type == STRING_TYPE) && (nptr->children[0]->type == STRING_TYPE)))
                {
                    nptr->type = BOOL_TYPE;
                    break;
                }
                else
                {
                    handle_error(ERR_TYPE);
                    return;
                }

            case TOK_TIMES: // *
                if ((nptr->children[0]->type == STRING_TYPE) && (nptr->children[1]->type == INT_TYPE))
                {
                    nptr->type = STRING_TYPE;
                    break;
                }
                else if ((nptr->children[0]->type == INT_TYPE) && (nptr->children[1]->type == INT_TYPE))
                {
                    nptr->type = INT_TYPE;
                    break;
                }
                else
                {
                    handle_error(ERR_TYPE);
                    return;
                }

            case TOK_PLUS: // +
                if ((nptr->children[0]->type == STRING_TYPE) && (nptr->children[1]->type == STRING_TYPE))
                {
                    nptr->type = STRING_TYPE;
                    break;
                }
                else if ((nptr->children[0]->type == INT_TYPE) && (nptr->children[1]->type == INT_TYPE))
                {
                    nptr->type = INT_TYPE;
                }
                else
                {
                    handle_error(ERR_TYPE);
                    return;
                }

            case TOK_BMINUS: // -
            case TOK_DIV:    // /
            case TOK_MOD:    // %
                if ((nptr->children[1]->type != INT_TYPE) || (nptr->children[0]->type != INT_TYPE))
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->type = INT_TYPE;
                break;

            case TOK_EQ: // ~
                if ((nptr->children[0]->type) != (nptr->children[1]->type))
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->type = BOOL_TYPE;
                break;

            case TOK_QUESTION: // ?
                if ((nptr->children[0]->type == BOOL_TYPE) && ((nptr->children[1]->type) != (nptr->children[2]->type)))
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->type = nptr->children[1]->type;
                break;

            case TOK_UMINUS: // _
                if (nptr->children[0]->type == INT_TYPE)
                {
                    nptr->type = INT_TYPE;
                }
                else if ((nptr->children[0]->type) == STRING_TYPE)
                {
                    nptr->type = STRING_TYPE;
                }
                else
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                break;

            case TOK_NOT: // !
                if ((nptr->children[0]->type) != BOOL_TYPE)
                {
                    handle_error(ERR_TYPE);
                    return;
                }
                nptr->type = BOOL_TYPE;
                break;

            default:
                break;
            }
        case NT_LEAF:
            if (nptr->type == ID_TYPE)
            {
                entry_t *entry = get(nptr->val.sval);
                free(nptr->val.sval);
                if (entry != NULL)
                {
                    nptr->type = entry->type;
                    if (nptr->type == INT_TYPE)
                    {
                        nptr->val.ival = entry->val.ival;
                    }
                    else if (nptr->type == BOOL_TYPE)
                    {
                        nptr->val.bval = entry->val.bval;
                    }
                    else
                    {
                        nptr->val.sval = (char *)malloc(strlen(entry->val.sval) + 1);
                        strcpy(nptr->val.sval, entry->val.sval);
                    }
                }
                else
                {
                    handle_error(ERR_UNDEFINED);
                }
                break;
            }
            break;
        default:
            break;
        }
    }
    return;
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 */

static void infer_root(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;

    // check for assignment
    if (nptr->type == ID_TYPE)
    {
        infer_type(nptr->children[1]);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL)
        {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO)
 */

static void eval_node(node_t *nptr)
{
    // check running status - you can ignore this.
    if (terminate || ignore_input)
        return;

    // Week 1 TODO: Implement a recursive post-order traversal of the AST. Remember to include a base case.

    if (nptr)
    {
        int numChildren = 3;
        for (int i = 0; i < numChildren; ++i)
        {
            if ((nptr->tok != TOK_QUESTION))
            {
                eval_node(nptr->children[i]);
            }
        }

        switch (nptr->node_type)
        {
        case NT_INTERNAL:
            // Week 1 TODO: Implement evaluation for all operators on int and bool types.
            // Week 2 TODO: Extend evaluation to handle operators on string types.
            if (is_unop(nptr->tok)) // IS IT A UNARY OPERATION
            {
                switch (nptr->tok) // IS IT A UNARY OPERATION
                {
                case TOK_NOT:
                    if (nptr->type == BOOL_TYPE)
                    {
                        nptr->val.bval = !(nptr->children[0]->val.bval);
                    }
                    break;
                case TOK_UMINUS:
                    if (nptr->type == INT_TYPE)
                    {
                        nptr->val.ival = nptr->children[0]->val.ival * -1;
                    }
                    else if (nptr->type == STRING_TYPE)
                    {
                        // nptr->val.sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + 1);
                        nptr->val.sval = strrev(nptr->children[0]->val.sval);
                    }
                    break;
                default:
                    break;
                }
            }
            if (is_binop(nptr->tok)) // IS IT A BINARY OPERATION
            {
                switch (nptr->tok)
                {
                case TOK_OR: //||
                    if (nptr->type == BOOL_TYPE)
                    {
                        nptr->val.bval = ((nptr->children[0]->val.bval) || (nptr->children[1]->val.bval));
                    }
                    break;
                case TOK_BMINUS: // -
                    if (nptr->type == INT_TYPE)
                    {
                        nptr->val.ival = ((nptr->children[0]->val.ival) - (nptr->children[1]->val.ival));
                    }
                    break;

                case TOK_PLUS: // +
                    if (nptr->type == INT_TYPE)
                    {
                        nptr->val.ival = ((nptr->children[0]->val.ival) + (nptr->children[1]->val.ival));
                    }
                    else if (nptr->type == STRING_TYPE)
                    {
                        nptr->val.sval = ((char *)malloc(strlen(nptr->children[0]->val.sval) + strlen(nptr->children[1]->val.sval) + 1));
                        strcpy(nptr->val.sval, (nptr->children[0]->val.sval));
                        strcat(nptr->val.sval, (nptr->children[1]->val.sval));
                    }
                    break;
                case TOK_DIV: // /
                    if (nptr->type == INT_TYPE)
                    {
                        if (nptr->children[1]->val.ival == 0)
                        {
                            handle_error(ERR_EVAL);
                            return;
                        }
                        nptr->val.ival = ((nptr->children[0]->val.ival) / (nptr->children[1]->val.ival));
                    }
                    break;
                case TOK_MOD: // %
                    if (nptr->type == INT_TYPE)
                    {
                        if ((nptr->children[1]->val.ival) == 0)
                        {
                            handle_error(ERR_EVAL);
                            return;
                        }
                        nptr->val.ival = ((nptr->children[0]->val.ival) % (nptr->children[1]->val.ival));
                    }
                    break;
                case TOK_TIMES: // *
                    if (nptr->type == INT_TYPE)
                    {
                        nptr->val.ival = ((nptr->children[0]->val.ival) * (nptr->children[1]->val.ival));
                    }
                    else if (nptr->type == STRING_TYPE)
                    {
                        if (nptr->children[1]->val.ival < 0)
                        {
                            handle_error(ERR_EVAL);
                            return;
                        }
                        int factor = (nptr->children[1]->val.ival);
                        nptr->val.sval = (char *)malloc((strlen(nptr->children[0]->val.sval) * (factor)) + 1);
                        nptr->val.sval[0] = '\0';
                        for (int i = 0; i < factor; i++)
                        {
                            strcat(nptr->val.sval, nptr->children[0]->val.sval);
                        }
                    }
                    break;

                case TOK_AND: //&
                    if (nptr->type == BOOL_TYPE)
                    {
                        nptr->val.bval = ((nptr->children[0]->val.bval) && (nptr->children[1]->val.bval));
                    }
                    break;

                case TOK_LT:
                    if (nptr->type == BOOL_TYPE)
                    {
                        bool val;
                        if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE)
                        {
                            val = ((nptr->children[0]->val.ival) < (nptr->children[1]->val.ival));
                        }
                        else if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE)
                        {
                            val = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) < 0);
                        }
                        nptr->val.bval = val;
                    }
                    break;

                case TOK_GT:
                    if (nptr->type == BOOL_TYPE)
                    {
                        bool val;
                        if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE)
                        {
                            val = ((nptr->children[0]->val.ival) > (nptr->children[1]->val.ival));
                        }
                        else if (nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE)
                        {
                            val = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) > 0);
                        }
                        nptr->val.bval = val;
                    }
                    break;

                case TOK_EQ:
                    if (nptr->type == BOOL_TYPE)
                    {
                        bool val;
                        if (nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE)
                        {
                            val = ((nptr->children[0]->val.ival) == (nptr->children[1]->val.ival));
                        }
                        else if (nptr->children[0]->type == BOOL_TYPE && nptr->children[1]->type == BOOL_TYPE)
                        {
                            val = ((nptr->children[0]->val.bval) == (nptr->children[1]->val.bval));
                        }
                        else
                        {
                            val = (strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) == 0);
                        }
                        nptr->val.bval = val;
                    }
                    break;

                default:
                    break;
                }
            }
            if (nptr->tok == TOK_QUESTION) // IS IT A TERNARY OPERATION
            {
                eval_node(nptr->children[0]);
                if (nptr->type == BOOL_TYPE)
                {
                    if (nptr->children[0]->val.bval)
                    {
                        eval_node(nptr->children[1]);
                        nptr->val.bval = nptr->children[1]->val.bval;
                    }
                    else
                    {
                        eval_node(nptr->children[2]);
                        nptr->val.bval = nptr->children[2]->val.bval;
                    }
                }
                else if (nptr->type == INT_TYPE)
                {
                    if (nptr->children[0]->val.bval)
                    {
                        eval_node(nptr->children[1]);
                        nptr->val.ival = nptr->children[1]->val.ival;
                    }
                    else
                    {
                        eval_node(nptr->children[2]);
                        nptr->val.ival = nptr->children[2]->val.ival;
                    }
                }

                else if (nptr->type == STRING_TYPE)
                {
                    if (nptr->children[0]->val.bval)
                    {
                        eval_node(nptr->children[1]);
                        nptr->val.sval = (char *)malloc(strlen(nptr->children[1]->val.sval) + 1);
                        strcpy(nptr->val.sval, nptr->children[1]->val.sval);
                    }
                    else
                    {
                        eval_node(nptr->children[2]);
                        nptr->val.sval = (char *)malloc(strlen(nptr->children[2]->val.sval) + 1);
                        strcpy(nptr->val.sval, nptr->children[2]->val.sval);
                    }
                }
            }
            // For reference, the identity (do-nothing) operator has been implemented for you.
            if (nptr->tok == TOK_IDENTITY)
            {
                if (nptr->type == STRING_TYPE)
                {
                    // Week 2 TODO: You'll need to make a copy of the string.
                    nptr->val.sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + 1);
                    strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                }
                else
                {
                    nptr->val.ival = nptr->children[0]->val.ival;
                }
            }
            break;
        case NT_LEAF:
            break;
        default:
            break;
        }
    }
    return;
}

/* eval_root() - set the value of the root node based on the values of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated.
 */

void eval_root(node_t *nptr)
{
    if (nptr == NULL)
        return;
    // check running status
    if (terminate || ignore_input)
        return;

    // check for assignment
    if (nptr->type == ID_TYPE)
    {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input)
            return;

        if (nptr->children[0] == NULL)
        {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i)
    {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input)
        return;

    if (nptr->type == STRING_TYPE)
    {
        (nptr->val).sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (!nptr->val.sval)
        {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    }
    else
    {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval()
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated.
 */

void infer_and_eval(node_t *nptr)
{
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str)
{
    // Week 2 TODO: Implement copying and reversing the string.
    int length = strlen(str);
    char *reversed = (char *)malloc(length + 1);
    for (int i = 0; i < length; i++)
    {
        reversed[i] = str[(length - 1) - i];
    }
    reversed[length] = '\0';

    return reversed;
}

/* cleanup() - frees the space allocated to the AST
 * Parameter: The node to free.
 */
void cleanup(node_t *nptr)
{
    if(nptr == NULL){
        return;
    }
    for(int i = 0; i < 3; i++){
        cleanup(nptr->children[i]);
    }
    if(nptr->type == STRING_TYPE || nptr -> type == ID_TYPE){
        free(nptr->val.sval);
    }
    // Week 2 TODO: Recursively free each node in the AST
    free(nptr);
    return;
}
