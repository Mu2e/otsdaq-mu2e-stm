from multiprocessing import Process, Queue
import atexit

from workers import worker_raw_data, worker_baseline, worker_peaks, worker_cpu, worker_alarms

class WorkerManager:
    def __init__(self):
        self._shutdown_done = False
        self.workers = []
        self.queues = {}

    def add_worker(self, name, worker_module, core_id):
        """Add worker of given type."""
        task_queue = Queue()
        result_queue = Queue()
        proc = Process(
                target = worker_module.run_worker,
                args = (task_queue, result_queue, core_id),
                daemon = True # worker exits if main exits
        )
        proc.start()
        self.workers.append(proc)
        self.queues[name] = (task_queue, result_queue)
        print(f"DQM::WorkerManager started worker '{name}' on core {core_id}")

    def get_queues(self, name):
        return self.queues.get(name, (None,None))

    def shutdown(self):
        print(f"DQM::WorkerManager shutting down all workers...")
        if self._shutdown_done:
            return

        # send shutdown signal
        for task_q, _ in self.queues.values():
            task_q.put(None)
        # join processes
        for proc in self.workers:
            proc.join(timeout=2)
            if proc.is_alive():
                print(f"DQM::WorkerManager did not exit cleanly, terminating...")
                proc.terminate()
                proc.join()
        
        self._shutdown_done = True
        print(f"DQM::WorkerManager all workers stopped.")


manager = WorkerManager()
atexit.register(manager.shutdown)
