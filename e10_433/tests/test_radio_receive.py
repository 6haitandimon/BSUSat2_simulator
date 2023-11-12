from radio_driver.radio_controller import RadioController


def test_radio_receive():
    controller = RadioController()

    if not controller.initialize():
        print("Test radio receive: failed to init controller")
        return

    if not controller.start_rx():
        print(print("Test radio receive: failed to start rx"))
        return
