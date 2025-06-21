/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de um pool de threads oferecido por Java */
/* -------------------------------------------------------------------*/

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicLong;

//classe runnable
class Worker implements Runnable {
    private final long steps;
    private static AtomicLong soma;

    //construtor
    Worker(long numSteps, AtomicLong soma) {
        this.soma = soma;
        this.steps = numSteps;
    }


    //método para execução
    public void run() {
        long s = 0;
        for (long i=1; i<this.steps; i++) {
            s += i;
        }
        soma.addAndGet(s);
        System.out.println(s);
    }
}

//classe do método main
public class AnotherHelloPool {
    private static final int NTHREADS = 10;
    private static final int WORKERS = 50;
    private static AtomicLong soma = new AtomicLong(0L);

    public static void main(String[] args) {
        //cria um pool de threads (NTHREADS)
        ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);

        //dispara a execução dos workers
        for (int i = 1; i < WORKERS; i++) {
            Runnable worker = new Worker(i, soma);
            executor.execute(worker);
        }
        //termina a execução das threads no pool (não permite que o executor
        // aceite novos objetos)
        executor.shutdown();

        //espera todas as threads terminarem
        while (!executor.isTerminated()) {}
        System.out.println("Soma:" + soma);
        System.out.println("Terminou");
    }
}
