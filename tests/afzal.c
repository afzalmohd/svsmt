int main()
{
	unsigned int x, y;

	__CPROVER_assume(x < 1000 && y < 5);
	if (x < 10) {
		x = x + y;
		x = x + 1;
	} else {
		y = y + 1;
	}

	assert(x <= 1000);

	return 0;
}


