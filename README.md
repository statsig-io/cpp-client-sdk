## Usage

Initialize the SDK

```c++
StatsigUser user;
user.user_id = "a-user";

// Create your own instance
StatsigClient client;
client.Initialize("client-{YOUR_CLIENT_SDK_KEY}", user);

// or, use the shared instance
StatsigClient::Shared().Initialize("client-{YOUR_CLIENT_SDK_KEY}", user);
```


Check a Gate

```c++
if (client.CheckGate("a_gate")) {
  // show new feature
}

// or

if (StatsigClient::Shared().CheckGate("a_gate")) {
  // show new feature
}
```


Get values from an Experiment

```c++
auto experiment = client.GetExperiment("an_experiment");

// or

auto experiment = StatsigClient::Shared().GetExperiment("an_experiment");

// then access the params
  
std::cout << experiment.GetValue()["a_string_param"] << std::endl;

```


Logging an Event

```c++
unordered_map<string, string> metadata(
    {
        {"is_verified", "yes"}
    }
);


client.LogEvent({"my_custom_event", 1.23, metadata});

// or

StatsigClient::Shared().LogEvent({"another_event", "string_values"});
```
