#include "main.h"
#include "support.h"
#include "model.h"
#include "render.h"
#include "logging.h"

void main(void)
{
      setup_devices();
      setup_model();
      setup_logging();

      params.num_iterations=4096;
      for (int i=0;i<params.num_iterations;i++) {
	      if (i==200)
		      model_enable_rivalism();
	      iteration();
	      save_image(i);
	      log_iteration(i);
      }
      done_logging();
}

