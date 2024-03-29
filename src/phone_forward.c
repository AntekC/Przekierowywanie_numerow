/** @file
 * Implementacja klasy przechowującej przekierowania numerów telefonicznych
 *
 * @author Antoni Bryk <ab438254@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 2022
 */
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "phone_forward.h"
#include "phone_numbers_operations.h"
#include "trie.h"

/**
 * To jest struktura przechowująca przekierowania numerów telefonów.
 * Składa się ona z dwóch dzrzew trie.
 */
struct PhoneForward {
    Trie *forward; ///< Wskaźnik na drzewo trie przekierowań.
    Trie *reverse; ///< Wskaźnik na drzewo trie odwróconych przekierowań.
};

/**
 * To jest struktura przechowująca ciąg numerów telefonów.
 * Ma ona postać listy.
 */
struct PhoneNumbers {
    struct PhoneNumbers *next; ///< Wskaźnik na następny element w liście.
    char *number; ///< Wskaźnik na napis reprezentujący numer telefonu.
};

/** @brief Wstawia numer na początek.
 * Wstawia numer @p number na początek struktury @p result, scalając go
 * z numerem @p num w zależności od poziomu @p level na jakim się znajdujemy w drzewie reverse.
 * @param[in,out] result – wskaźnik na strukturę przechowującą ciąg numerów telefonów;
 * @param[in] number – wskaźnik na napis reprezentujący numer do wstawienia;
 * @param[in] level - głebokość w jakiej jesteśmy w drzewie reverse.
 * @param[in] num - wskaźnik na napis reprezentujący numer do scalenia.
 * @return Wartość @p true, jeśli udało się wstawić numer.
 *         Wartość @p false, jeśli nie udało się alokować pamięci.
 */
static bool insertFirstNumber(PhoneNumbers **result, char const *number, size_t level, char const *num) {
    char *number_to_insert = combineNumbers(number, num + level + 1);
    if (number_to_insert == NULL) {
        return false;
    }

    // Już jest taki numer w wyniku zatem nie trzeba go dodawać.
    if (areNumbersIndentical(number_to_insert, (*result)->number)) {
        free(number_to_insert);
        return true;
    }

    // Wstawiamy scalony numer do struktury.
    if ((*result) != NULL) {
        PhoneNumbers *new = newPhoneNumber(number_to_insert);
        if (new == NULL) {
            free(number_to_insert);
            return false;
        }
        new->next = (*result);
        (*result) = new;
        return true;
    } else {
        return false;
    }
}

/** @brief Wstawia numer do struktury.
 * Wstawia numer @p number na nie na początek struktury @p result, scalając go
 * z numerem @p num w zależności od poziomu @p level na jakim się znajdujemy w drzewie reverse.
 * @param[in,out] result – wskaźnik na strukturę przechowującą ciąg numerów telefonów;
 * @param[in] number – wskaźnik na napis reprezentujący numer do wstawienia;
 * @param[in] level - głebokość w jakiej jesteśmy w drzewie reverse.
 * @param[in] num - wskaźnik na napis reprezentujący numer do scalenia.
 * @return Wartość @p true, jeśli udało się wstawić numer.
 *         Wartość @p false, jeśli nie udało się alokować pamięci.
 */
static bool insertLaterNumber(PhoneNumbers *result, char const *number, size_t level, char const *num) {
    char *number_to_insert = combineNumbers(number, num + level + 1);
    if (number_to_insert == NULL) {
        return false;
    }
    PhoneNumbers *insert = newPhoneNumber(number_to_insert);
    if (insert == NULL) {
        free(number_to_insert);
        return false;
    }

    PhoneNumbers *prev = result;
    result = result->next;

    while (result != NULL) {
        if (areNumbersIndentical(result->number, number_to_insert)) {
            phnumDelete(insert);
            return true;
        } else if (compareNumbers(result->number, number_to_insert)) {
            prev->next = insert;
            insert->next = result;
            return true;
        } else {
            prev = result;
            result = result->next;
        }
    }

    prev->next = insert;
    return true;
}


/** @brief Usuwa numery z struktury.
 * Usuwa numery ze struktury @p ans, które w wyniku wywołania przez funkcje phfwdGet
 * nie zwracją numeru @p num.
 * @param[in,out] ans – wskaźnik na strukturę przechowującą ciąg numerów telefonów;
 * @param[in] pf – wskaźnik na strukturę przechowującą przekierowania numerów;
 * @param[in] num – wskaźnik na napis reprezentujący numer do sprawdzenia;
 * @return Wartość @p true, jeśli udało się usunąć numery.
 *         Wartość @p false, jeśli nie udało się alokować pamięci przy sprawdzaniu numerów.
 */
static bool deleteNumbersFromReversePhnum(PhoneNumbers **ans, PhoneForward const *pf, char const *num) {
    if((*ans) == NULL){
        return true;
    }

    PhoneNumbers *jumper = NULL;
    PhoneNumbers *before = NULL;

    while((*ans) != NULL){
        PhoneNumbers *buffer;
        buffer = phfwdGet(pf,(*ans)->number);
        if(buffer == NULL){
            return false;
        }

        if(areNumbersIndentical(num, buffer->number)){
            phnumDelete(buffer);
            buffer = NULL;

            before = (*ans);
            jumper = before->next;
            break;
        } else {
            phnumDelete(buffer);
            buffer = NULL;

            PhoneNumbers *save = (*ans);
            (*ans) = (*ans)->next;

            free(save->number);
            free(save);
        }
    }

    if((*ans) == NULL || before == NULL){
        return true;
    }

    while(jumper != NULL){
        PhoneNumbers *buffer;
        buffer = phfwdGet(pf,jumper->number);
        if(buffer == NULL){
            return false;
        }

        if(areNumbersIndentical(num, buffer->number)){
            phnumDelete(buffer);
            buffer = NULL;

            before = jumper;
            jumper = jumper->next;
        } else {
            phnumDelete(buffer);
            buffer = NULL;

            PhoneNumbers *save = jumper;
            jumper = jumper->next;
            before->next = jumper;

            free(save->number);
            free(save);
        }
    }

    return true;
}

PhoneNumbers *newPhoneNumber(char *number) {
    PhoneNumbers *ans = malloc(sizeof(PhoneNumbers));
    if (ans == NULL) {
        return NULL;
    }

    ans->number = number;
    ans->next = NULL;

    return ans;
}
PhoneForward *phfwdNew(void) {
    PhoneForward *ans = malloc(sizeof(PhoneForward));
    if (ans == NULL) {
        return NULL;
    }

    ans->forward = newNode();
    if (ans->forward == NULL) {
        free(ans);
        return NULL;
    }

    ans->reverse = newNode();
    if (ans->reverse == NULL) {
        free(ans->forward);
        free(ans);
        return NULL;
    }

    return ans;
}


void changeFirstNumber(PhoneNumbers *pnum, char *num) {
    free(pnum->number);
    pnum->number = num;
}

bool addNextNumber(PhoneNumbers *pnum, char *num) {
    if (pnum != NULL) {
        while (pnum->next != NULL) {
            pnum = pnum->next;
        }
        pnum->next = newPhoneNumber(num);
        if (pnum->next == NULL) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void phnumDeleteFirstNumber(PhoneNumbers **pnum) {
    if ((*pnum) != NULL) {
        PhoneNumbers *save = *pnum;
        (*pnum) = (*pnum)->next;
        free(save->number);
        free(save);
    }
}

void phnumDeleteLaterNumber(PhoneNumbers *pnum, char const *number) {
    if (pnum != NULL) {
        PhoneNumbers *prev = pnum;
        pnum = pnum->next;

        while (pnum != NULL) {
            if (areNumbersIndentical(pnum->number, number)) {
                prev->next = pnum->next;

                free(pnum->number);
                free(pnum);
                break;
            } else {
                prev = pnum;
                pnum = pnum->next;
            }
        }
    }
}

void phnumDeleteAllLaterNumbersStarting(PhoneNumbers *pnum, char const *prefix) {
    if(pnum != NULL) {
        PhoneNumbers *prev = pnum;
        pnum = pnum->next;

        while (pnum != NULL) {
            if (startsWith(pnum->number, prefix)) {
                prev->next = pnum->next;
                PhoneNumbers *save = pnum;
                pnum = pnum->next;

                free(save->number);
                free(save);
            } else {
                prev = pnum;
                pnum = pnum->next;
            }
        }
    }
}

bool addNumbers(PhoneNumbers *source, PhoneNumbers **target, size_t level, char const *num) {
    while (source != NULL) {
        char *number_to_compare = combineNumbers(source->number, num + level + 1);

        if (number_to_compare == NULL){
            return false;
        }

        if (compareNumbers((*target)->number, number_to_compare)) {
            free(number_to_compare);
            if (!insertFirstNumber(target, source->number, level, num)) {
                return false;
            }
        } else {
            free(number_to_compare);
            if (!insertLaterNumber(*target, source->number, level, num)) {
                return false;
            }
        }
        source = source->next;
    }
    return true;
}

void phfwdDelete(PhoneForward *pf) {
    if (pf != NULL) {
        if (pf->forward != NULL) {
            deleteTrie(pf->forward);
        }
        if (pf->reverse != NULL) {
            deleteTrie(pf->reverse);
        }
        free(pf);
    }
}

bool phfwdAdd(PhoneForward *pf, char const *num1, char const *num2) {
    if (pf == NULL) {
        return false;
    } else {
        return (insert(pf->forward, pf->reverse, num1, num2, false) &&
                insert(pf->reverse, pf->reverse, num2, num1, true));
    }
}

void phfwdRemove(PhoneForward *pf, char const *num) {
    if (pf != NULL) {
        removeFromForward(pf->forward, pf->reverse, num);
    }
}

void phnumDelete(PhoneNumbers *pnum) {
    while (pnum != NULL) {
        PhoneNumbers *save = pnum;
        pnum = pnum->next;
        free(save->number);
        free(save);
    }
}

PhoneNumbers *phfwdGet(PhoneForward const *pf, char const *num) {
    PhoneNumbers *ans = NULL;

    if (pf != NULL && pf->forward != NULL) {
        getFromForward(pf->forward, num, &ans);
    }

    return ans;
}

char const *phnumGet(PhoneNumbers const *pnum, size_t idx) {
    for (size_t i = 0; i < idx; ++i) {
        if (pnum != NULL) {
            pnum = pnum->next;
        } else {
            break;
        }
    }

    if (pnum == NULL) {
        return NULL;
    } else {
        return pnum->number;
    }
}

PhoneNumbers *phfwdReverse(PhoneForward const *pf, char const *num) {
    PhoneNumbers *ans = NULL;

    if (pf != NULL) {
        getFromReverse(pf->reverse, num, &ans);
        return ans;
    } else {
        return NULL;
    }
}

PhoneNumbers * phfwdGetReverse(PhoneForward const *pf, char const *num){
    PhoneNumbers *ans = NULL;

    if(pf != NULL){
        getFromReverse(pf->reverse, num, &ans);
        if (deleteNumbersFromReversePhnum(&ans, pf, num)){
            if(ans == NULL){
                return newPhoneNumber(NULL);
            } else {
                return ans;
            }
        } else {
            phnumDelete(ans);
            return NULL;
        }

    } else {
        return NULL;
    }
}


