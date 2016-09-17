int main() {
	if (!ifusb_init())
		goto out;

out:
	ifusb_close();

}