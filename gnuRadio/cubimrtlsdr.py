#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: receiver
# Author: Vasilina
# GNU Radio version: 3.10.9.2

from PyQt5 import Qt
from gnuradio import qtgui
from PyQt5 import QtCore
from gnuradio import analog
from gnuradio import blocks
from gnuradio import blocks, gr
from gnuradio import digital
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import network
import osmosdr
import time
import sip



class cubimrtlsdr(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "receiver", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("receiver")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "cubimrtlsdr")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)

        ##################################################
        # Variables
        ##################################################
        self.sig_freq = sig_freq = 433847000
        self.sig_baudrate = sig_baudrate = 9600
        self.sdr_freq_offset = sdr_freq_offset = 0
        self.fir_samp_per_sym = fir_samp_per_sym = 2
        self.sdr_samp_rate = sdr_samp_rate = 2048000
        self.sdr_freq = sdr_freq = sig_freq+sdr_freq_offset
        self.fir_samp_rate = fir_samp_rate = sig_baudrate*fir_samp_per_sym
        self.fir_freq_correction = fir_freq_correction = 0
        self.sync_gain_mu = sync_gain_mu = 0.175
        self.fir_transition_bw = fir_transition_bw = sig_baudrate
        self.fir_offset = fir_offset = sig_freq-sdr_freq+fir_freq_correction
        self.fir_decimation = fir_decimation = sdr_samp_rate//fir_samp_rate
        self.sync_omega_limit = sync_omega_limit = 0.01
        self.sync_mu = sync_mu = 0.3
        self.sync_gain_omega = sync_gain_omega = 0.5 * sync_gain_mu * sync_gain_mu
        self.squelch_threshold = squelch_threshold = -58
        self.sdr_samp_per_sym = sdr_samp_per_sym = sdr_samp_rate/sig_baudrate
        self.fir_taps = fir_taps =  firdes.low_pass(1,sdr_samp_rate,sdr_samp_rate/(2*fir_decimation), fir_transition_bw)
        self.fir_freq = fir_freq = sdr_freq+fir_offset

        ##################################################
        # Blocks
        ##################################################

        self._squelch_threshold_range = qtgui.Range(-100, +0, 1, -58, 200)
        self._squelch_threshold_win = qtgui.RangeWidget(self._squelch_threshold_range, self.set_squelch_threshold, "Squelch threshold (dB)", "dial", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._squelch_threshold_win, 4, 0, 1, 2)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._sdr_freq_offset_range = qtgui.Range(-500000, +500000, 1000, 0, 200)
        self._sdr_freq_offset_win = qtgui.RangeWidget(self._sdr_freq_offset_range, self.set_sdr_freq_offset, "SDR carrier frequency shift (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._sdr_freq_offset_win, 0, 0, 1, 2)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.rtlsdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + ""
        )
        self.rtlsdr_source_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0.set_sample_rate(sdr_samp_rate)
        self.rtlsdr_source_0.set_center_freq(sig_freq, 0)
        self.rtlsdr_source_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0.set_gain(10, 0)
        self.rtlsdr_source_0.set_if_gain(20, 0)
        self.rtlsdr_source_0.set_bb_gain(20, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)
        self.qtgui_waterfall_sink_x_0_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            fir_freq, #fc
            fir_samp_rate, #bw
            "XLAT WATERFALL", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_0_0.set_update_time(0.10)
        self.qtgui_waterfall_sink_x_0_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_0_0.enable_axis_labels(True)



        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_0_0.set_intensity_range(-80, -10)

        self._qtgui_waterfall_sink_x_0_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_0_win, 3, 0, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_waterfall_sink_x_0 = qtgui.waterfall_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            sdr_freq, #fc
            sdr_samp_rate, #bw
            "SDR WATERFALL", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_waterfall_sink_x_0.set_update_time(0.10)
        self.qtgui_waterfall_sink_x_0.enable_grid(True)
        self.qtgui_waterfall_sink_x_0.enable_axis_labels(True)



        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_0.set_intensity_range(-80, -10)

        self._qtgui_waterfall_sink_x_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0_0 = qtgui.freq_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            fir_freq, #fc
            fir_samp_rate, #bw
            "XLAT SPECTRE", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_0_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0_0.set_y_axis((-140), 10)
        self.qtgui_freq_sink_x_0_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0_0.enable_grid(True)
        self.qtgui_freq_sink_x_0_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0_0.enable_control_panel(False)
        self.qtgui_freq_sink_x_0_0.set_fft_window_normalized(False)



        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_0_win, 3, 1, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
            1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            sdr_freq, #fc
            sdr_samp_rate, #bw
            "SDR SPECTRE", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis((-140), 10)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(True)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)
        self.qtgui_freq_sink_x_0.set_fft_window_normalized(False)



        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.network_socket_pdu_0 = network.socket_pdu('UDP_CLIENT', '127.0.0.1', '9999', 10000, False)
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccf(fir_decimation, fir_taps, fir_offset, sdr_samp_rate)
        self._fir_freq_correction_range = qtgui.Range(-3000, +3000, 100, 0, 200)
        self._fir_freq_correction_win = qtgui.RangeWidget(self._fir_freq_correction_range, self.set_fir_freq_correction, "Frequency shift (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._fir_freq_correction_win, 2, 0, 1, 2)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.digital_hdlc_deframer_bp_0 = digital.hdlc_deframer_bp(32, 500)
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
            samples_per_symbol=2,
            sensitivity=1.0,
            gain_mu=sync_gain_mu,
            mu=sync_mu,
            omega_relative_limit=sync_omega_limit,
            freq_error=0.0,
            verbose=False,
            log=False)
        self.digital_diff_decoder_bb_0 = digital.diff_decoder_bb(2, digital.DIFF_NRZI)
        self.digital_descrambler_bb_0 = digital.descrambler_bb(0x21, 0x0, 16)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, sdr_samp_rate,True)
        self.blocks_message_debug_0 = blocks.message_debug(True, gr.log_levels.info)
        self.analog_simple_squelch_cc_0 = analog.simple_squelch_cc(squelch_threshold, 1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.digital_hdlc_deframer_bp_0, 'out'), (self.blocks_message_debug_0, 'print'))
        self.msg_connect((self.digital_hdlc_deframer_bp_0, 'out'), (self.network_socket_pdu_0, 'pdus'))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))
        self.connect((self.digital_descrambler_bb_0, 0), (self.digital_hdlc_deframer_bp_0, 0))
        self.connect((self.digital_diff_decoder_bb_0, 0), (self.digital_descrambler_bb_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.digital_diff_decoder_bb_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.analog_simple_squelch_cc_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.qtgui_freq_sink_x_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.qtgui_waterfall_sink_x_0_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.qtgui_waterfall_sink_x_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "cubimrtlsdr")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_sig_freq(self):
        return self.sig_freq

    def set_sig_freq(self, sig_freq):
        self.sig_freq = sig_freq
        self.set_fir_offset(self.sig_freq-self.sdr_freq+self.fir_freq_correction)
        self.set_sdr_freq(self.sig_freq+self.sdr_freq_offset)
        self.rtlsdr_source_0.set_center_freq(self.sig_freq, 0)

    def get_sig_baudrate(self):
        return self.sig_baudrate

    def set_sig_baudrate(self, sig_baudrate):
        self.sig_baudrate = sig_baudrate
        self.set_fir_samp_rate(self.sig_baudrate*self.fir_samp_per_sym)
        self.set_fir_transition_bw(self.sig_baudrate)
        self.set_sdr_samp_per_sym(self.sdr_samp_rate/self.sig_baudrate)

    def get_sdr_freq_offset(self):
        return self.sdr_freq_offset

    def set_sdr_freq_offset(self, sdr_freq_offset):
        self.sdr_freq_offset = sdr_freq_offset
        self.set_sdr_freq(self.sig_freq+self.sdr_freq_offset)

    def get_fir_samp_per_sym(self):
        return self.fir_samp_per_sym

    def set_fir_samp_per_sym(self, fir_samp_per_sym):
        self.fir_samp_per_sym = fir_samp_per_sym
        self.set_fir_samp_rate(self.sig_baudrate*self.fir_samp_per_sym)

    def get_sdr_samp_rate(self):
        return self.sdr_samp_rate

    def set_sdr_samp_rate(self, sdr_samp_rate):
        self.sdr_samp_rate = sdr_samp_rate
        self.set_fir_decimation(self.sdr_samp_rate//self.fir_samp_rate)
        self.set_fir_taps( firdes.low_pass(1,self.sdr_samp_rate,self.sdr_samp_rate/(2*self.fir_decimation), self.fir_transition_bw))
        self.set_sdr_samp_per_sym(self.sdr_samp_rate/self.sig_baudrate)
        self.blocks_throttle_0.set_sample_rate(self.sdr_samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.sdr_freq, self.sdr_samp_rate)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.sdr_freq, self.sdr_samp_rate)
        self.rtlsdr_source_0.set_sample_rate(self.sdr_samp_rate)

    def get_sdr_freq(self):
        return self.sdr_freq

    def set_sdr_freq(self, sdr_freq):
        self.sdr_freq = sdr_freq
        self.set_fir_freq(self.sdr_freq+self.fir_offset)
        self.set_fir_offset(self.sig_freq-self.sdr_freq+self.fir_freq_correction)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.sdr_freq, self.sdr_samp_rate)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.sdr_freq, self.sdr_samp_rate)

    def get_fir_samp_rate(self):
        return self.fir_samp_rate

    def set_fir_samp_rate(self, fir_samp_rate):
        self.fir_samp_rate = fir_samp_rate
        self.set_fir_decimation(self.sdr_samp_rate//self.fir_samp_rate)
        self.qtgui_freq_sink_x_0_0.set_frequency_range(self.fir_freq, self.fir_samp_rate)
        self.qtgui_waterfall_sink_x_0_0.set_frequency_range(self.fir_freq, self.fir_samp_rate)

    def get_fir_freq_correction(self):
        return self.fir_freq_correction

    def set_fir_freq_correction(self, fir_freq_correction):
        self.fir_freq_correction = fir_freq_correction
        self.set_fir_offset(self.sig_freq-self.sdr_freq+self.fir_freq_correction)

    def get_sync_gain_mu(self):
        return self.sync_gain_mu

    def set_sync_gain_mu(self, sync_gain_mu):
        self.sync_gain_mu = sync_gain_mu
        self.set_sync_gain_omega(0.5 * self.sync_gain_mu * self.sync_gain_mu)

    def get_fir_transition_bw(self):
        return self.fir_transition_bw

    def set_fir_transition_bw(self, fir_transition_bw):
        self.fir_transition_bw = fir_transition_bw
        self.set_fir_taps( firdes.low_pass(1,self.sdr_samp_rate,self.sdr_samp_rate/(2*self.fir_decimation), self.fir_transition_bw))

    def get_fir_offset(self):
        return self.fir_offset

    def set_fir_offset(self, fir_offset):
        self.fir_offset = fir_offset
        self.set_fir_freq(self.sdr_freq+self.fir_offset)
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(self.fir_offset)

    def get_fir_decimation(self):
        return self.fir_decimation

    def set_fir_decimation(self, fir_decimation):
        self.fir_decimation = fir_decimation
        self.set_fir_taps( firdes.low_pass(1,self.sdr_samp_rate,self.sdr_samp_rate/(2*self.fir_decimation), self.fir_transition_bw))

    def get_sync_omega_limit(self):
        return self.sync_omega_limit

    def set_sync_omega_limit(self, sync_omega_limit):
        self.sync_omega_limit = sync_omega_limit

    def get_sync_mu(self):
        return self.sync_mu

    def set_sync_mu(self, sync_mu):
        self.sync_mu = sync_mu

    def get_sync_gain_omega(self):
        return self.sync_gain_omega

    def set_sync_gain_omega(self, sync_gain_omega):
        self.sync_gain_omega = sync_gain_omega

    def get_squelch_threshold(self):
        return self.squelch_threshold

    def set_squelch_threshold(self, squelch_threshold):
        self.squelch_threshold = squelch_threshold
        self.analog_simple_squelch_cc_0.set_threshold(self.squelch_threshold)

    def get_sdr_samp_per_sym(self):
        return self.sdr_samp_per_sym

    def set_sdr_samp_per_sym(self, sdr_samp_per_sym):
        self.sdr_samp_per_sym = sdr_samp_per_sym

    def get_fir_taps(self):
        return self.fir_taps

    def set_fir_taps(self, fir_taps):
        self.fir_taps = fir_taps
        self.freq_xlating_fir_filter_xxx_0.set_taps(self.fir_taps)

    def get_fir_freq(self):
        return self.fir_freq

    def set_fir_freq(self, fir_freq):
        self.fir_freq = fir_freq
        self.qtgui_freq_sink_x_0_0.set_frequency_range(self.fir_freq, self.fir_samp_rate)
        self.qtgui_waterfall_sink_x_0_0.set_frequency_range(self.fir_freq, self.fir_samp_rate)




def main(top_block_cls=cubimrtlsdr, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
