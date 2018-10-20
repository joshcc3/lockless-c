
int main()
{
  __int128_t a;
  __atomic_compare_exchange(a, a, a, 0);
}
