int main(int argc, char **argv)
{
  int a = 1, b = -4, c = 0;

  int positive = a * a;
  positive += a + a;
  positive += c;

  int zero = a * c;
  zero += b * c;
  zero += c;
  zero -= c;

  int negative = b * a;
  negative += b;
  negative += c - c;

  int positive_negative = a + b;
  positive_negative *= b;

  return 0;
}
