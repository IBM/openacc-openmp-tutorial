
.PHONY: all clean

TARGETS=leadership-graph.png

all: $(TARGETS)

%.png: %.dot
	dot $< -Tpng -o $@

clean:
	rm -f $(TARGETS)
