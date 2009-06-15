//quicksort sequencial
void qsort_seq(int array[], int begin, int end) {
   if(end - begin > 0) {
    int aux;
    int pivot = array[begin];
    int left = begin + 1;
    int right = end;
    while(left < right) {
        if(array[left] <= pivot) {
            left++;
        } else {
           // Troca o valor de array[left] com array[right]
           aux = array[left];
           array[left] = array[right];
           array[right] = aux;
           // Fim da troca ( equivale a fun��o swap(array[left], array[right]) )
           right--;
        }
    }
    if(array[left] > pivot) {
        left--;
    }
                                         
    // Troca o valor de array[begin] com array[left]
    aux = array[begin];
    array[begin] = array[left];
    array[left] = aux;
    // Fim da troca ( equivale a fun��o swap(array[begin], array[left]) )
    // Faz as chamadas recursivas para as duas partes da lista
    qsort_seq(array, begin, left-1);
    qsort_seq(array, right, end);
   }
}

