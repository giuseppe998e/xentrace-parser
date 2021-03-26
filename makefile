CC = gcc
CFLAGS = -Os -s
CINCLD = -I $(LIBDIR)

CP = cp
RM = rm -f
MKD = mkdir

LIBDIR = ./lib
SRCDIR = ./src
OUTDIR = ./out

#---
.PHONY: build
build: $(OUTDIR)/xentrace-parser.o
	@$(CP) $(SRCDIR)/xentrace-parser.h $(OUTDIR)
	@$(CP) $(SRCDIR)/xentrace-event.h $(OUTDIR)

# ---
$(OUTDIR)/%.o: $(SRCDIR)/%.c
	@$(MKD) -p $(OUTDIR)
	@$(CC) $(CFLAGS) $(CINCLD) -c $< -o $@

# ---
.PHONY: clean
clean:
	@$(RM) -r $(OUTDIR)