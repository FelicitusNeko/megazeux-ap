# MZX Test Worlds

This folder contains a set of worlds that test various aspects of MZX and
particularly Robotic that can be determined to be correct or incorrect from
within MegaZeux itself. Run the full sequence of tests with `run.sh`, or
use the `make test` rule.

This system currently requires an `mzxrun` executable in the parent directory.
Currently, the only applicable target architectures are `win64`, `win32`, and
`unix-devel`. These tests will NOT WORK with any other architecture!

## Conventions

### Naming

Test worlds are named in the format

```VVV #XXXXX [short description].mzx```

where `VVV` is the MegaZeux version associated with the world (e.g. `2.51`)
and #XXXXX is the test number (e.g. `#12345`).

Example version strings:
* `2.02` for 2.02
* `2.07` for 2.07
* `2.51` for 2.51
* `2.51s1` for 2.51s1
* `2.51s2` for 2.51s2
* `2.51s3` for 2.51s3
* `2.51s3.1` for 2.51s3.1
* `2.51s3.2` for 2.51s3.2
* `2.60` for 2.6
* `2.61` for 2.61
* `2.62` for 2.62
* `2.62b` for 2.62b
* etc...

### Robotic

Additionally, test worlds must conform to the following conventions:

1) The world must not be encrypted, and must have a properly configured
starting board. A title is helpful but not necessary.

2) The world will perform exactly one test or a group of closely related tests
(e.g. testing various COPY BLOCK uses) and will be created and versioned for the
earliest possible MegaZeux version it is applicable for. If versioned compatibility
behavior exists, create a version of the test for each behavior with similar
filenames and titles using the earliest applicable MZX versions for each.

3) The test must assume it will be operating at MZX_SPEED 1 and with unbounded
COMMANDS, unless the purpose of the test requires a different MZX_SPEED or
COMMANDS value. Setting these defaults explicitly is not necessary.

4) The robot driving the tests must be clearly visible.

5) The first line(s) of the testing robot must be setting the `$title` string
to the title of the test, the `$author` string to your identifier, and the
`$desc` string to a description of the test. This must be wrapped to fit the
robot editor window with `inc "$desc" "[more description]"` as-needed.
(EXCEPTION: MegaZeux worlds from versions under 2.80 are not capable of this.
Include this information in comments instead.)

6) Upon completion or failure, the counter `result` must be set to the
counter `PASS` or the counter `FAIL`.

7) Extra testing notes may be included in the `$result` string.
(EXCEPTION: MegaZeux worlds from versions under 2.80 are not capable of this.)

8) Counter and string names beginning with two underscores (e.g. `__abc`,
`$__def`) are reserved and should not be used.

9) When the test is finished running, the following snippet of code
MUST be executed:

```
if "__continue" = 1 then "__swap"
end
: "__swap"
swap world "next"
```