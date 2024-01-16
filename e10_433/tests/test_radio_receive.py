from radio_driver.radio_controller import RadioController


def test_radio_receive():
    controller = RadioController()

    if controller.initialize():
        print("Test radio receive: failed to init controller")
        return

    if controller.start_rx():
        print(print("Test radio receive: failed to start rx"))
        return
