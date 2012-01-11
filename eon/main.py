#!/usr/bin/env python
import os
from cStringIO import StringIO

import akmc
import basinhopping
import config
import parallelreplica

def main():
    config.init()
    job = config.main_job.lower()
    if job == 'akmc':
        akmc.main()
    elif job == 'parallel_replica':
        parallelreplica.main()
    elif job == 'basin_hopping':
        basinhopping.main()
    else:
        import communicator
        import shutil
        config.path_scratch = config.path_root
        comm = communicator.get_communicator()

        invariants = {}
        job = {}
        files = [ f for f in os.listdir(".") if os.path.isfile(f) ]
        for f in files:
            fh = open(f)
            f_passed = f.split('.')[0] + "_passed." + f.split('.')[1]
            job[f_passed] = StringIO(fh.read())
            fh.close()
        job["id"] = "output"
        if os.path.isdir("output_old"):
            shutil.rmtree("output_old")
        if os.path.isdir("output"):
            shutil.move("output", "output_old")
        comm.submit_jobs([job], invariants)

if __name__ == '__main__':
    main()