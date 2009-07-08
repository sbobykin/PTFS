static struct read_op_context {
	char* path;
	char* buf;
	size_t size;
	off_t offset;
};

int ptfs_read_from_parsed(struct read_op_context* read_op_context);
int ptfs_read_from_text(struct read_op_context* read_op_context);
